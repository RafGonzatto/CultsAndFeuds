#include "Navigation/EnemyFlowFieldProcessor.h"

#include "MassExecutionContext.h"
#include "MassProcessingPhaseManager.h"
#include "Engine/World.h"
#include "MassEntitySubsystem.h"
#include "MassEntityManager.h"
#include "Fragments/EnemyAgentFragment.h"
#include "Fragments/EnemyTargetFragment.h"
#include "Services/EnemyPerceptionService.h"
#include "Systems/EnemyMassSystem.h"
#include "Controllers/EnemyMassController.h"

namespace EnemyFlowField
{
    static int32 ComputeGridExtent(float GridSize, float CellSize)
    {
        const float HalfGrid = GridSize * 0.5f;
        return FMath::Max(1, FMath::CeilToInt(HalfGrid / CellSize));
    }
}

// FFlowField is declared in the public header as a public nested struct

UEnemyFlowFieldProcessor::UEnemyFlowFieldProcessor()
{
    EntityQuery.RegisterWithProcessor(*this);
    ExecutionOrder.ExecuteInGroup = FName(TEXT("Behavior"));
    bRequiresGameThreadExecution = false;
    ProcessorRequirements.AddSubsystemRequirement<UMassEntitySubsystem>(EMassFragmentAccess::ReadOnly);
}

void UEnemyFlowFieldProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    (void)EntityManager;
    EntityQuery.AddRequirement<FEnemyAgentFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FEnemyTargetFragment>(EMassFragmentAccess::ReadWrite);
}

void UEnemyFlowFieldProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    const UMassEntitySubsystem* EntitySubsystem = Context.GetSubsystem<UMassEntitySubsystem>();

    if (!PerceptionService && EntitySubsystem)
    {
        if (UWorld* World = EntitySubsystem->GetWorld())
        {
            if (UEnemyMassSystem* MassSystem = World->GetSubsystem<UEnemyMassSystem>())
            {
                if (UEnemyMassController* Controller = MassSystem->GetController())
                {
                    PerceptionService = Controller->GetPerceptionService();
                }
            }
        }
    }

    if (!PerceptionService || !PerceptionService->HasValidPrimaryTarget())
    {
        return;
    }

    const FVector PrimaryTargetLocation = PerceptionService->GetPrimaryTargetLocation();
    TimeSinceLastUpdate += Context.GetDeltaTimeSeconds();

    const float DistanceSquared = FVector::DistSquared(LastFlowFieldCenter, PrimaryTargetLocation);
    if (DistanceSquared > FMath::Square(FlowFieldUpdateDistance) || TimeSinceLastUpdate >= FlowFieldUpdateInterval)
    {
        LastFlowFieldCenter = PrimaryTargetLocation;
        RequestFlowField(PrimaryTargetLocation);
        TimeSinceLastUpdate = 0.0f;
    }

    ApplyFlowField(EntityManager, Context);
}

void UEnemyFlowFieldProcessor::RequestFlowField(const FVector& Center)
{
    // Compute synchronously for bring-up
    TSharedPtr<FFlowField> NewField = MakeShared<FFlowField>();
    NewField->Center = Center;
    NewField->CellSize = CellSize;
    NewField->HalfExtent = EnemyFlowField::ComputeGridExtent(GridSize, CellSize);

    const int32 GridWidth = NewField->HalfExtent * 2;
    NewField->Directions.SetNum(GridWidth * GridWidth);

    for (int32 Y = -NewField->HalfExtent; Y < NewField->HalfExtent; ++Y)
    {
        for (int32 X = -NewField->HalfExtent; X < NewField->HalfExtent; ++X)
        {
            const int32 LocalX = X + NewField->HalfExtent;
            const int32 LocalY = Y + NewField->HalfExtent;
            const int32 Index = LocalY * GridWidth + LocalX;

            const FVector CellWorldPos = Center + FVector(X * CellSize + CellSize * 0.5f, Y * CellSize + CellSize * 0.5f, 0.0f);
            FVector Direction = (Center - CellWorldPos).GetSafeNormal();
            NewField->Directions[Index] = Direction;
        }
    }

    ActiveFlowField = NewField;
    bFlowFieldDirty = false;
}

void UEnemyFlowFieldProcessor::ApplyFlowField(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    if (!ActiveFlowField.IsValid() || bFlowFieldDirty)
    {
        return;
    }

    EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& ChunkContext)
    {
        TArrayView<FEnemyAgentFragment> AgentFragments = ChunkContext.GetMutableFragmentView<FEnemyAgentFragment>();
        TArrayView<FEnemyTargetFragment> TargetFragments = ChunkContext.GetMutableFragmentView<FEnemyTargetFragment>();

        for (int32 EntityIndex = 0; EntityIndex < ChunkContext.GetNumEntities(); ++EntityIndex)
        {
            FEnemyAgentFragment& AgentFragment = AgentFragments[EntityIndex];
            FEnemyTargetFragment& TargetFragment = TargetFragments[EntityIndex];

            FVector FlowDirection = ActiveFlowField->GetFlowAtPosition(AgentFragment.Position);
            if (!FlowDirection.IsNearlyZero())
            {
                const float DistToTarget = FVector::Distance(AgentFragment.Position, TargetFragment.TargetLocation);
                const float DirectPathWeight = (DirectPathThreshold <= 0.0f)
                    ? 0.0f
                    : FMath::Clamp(1.0f - DistToTarget / DirectPathThreshold, 0.0f, 1.0f);

                if (DirectPathWeight > 0.0f)
                {
                    const FVector DirectPath = (TargetFragment.TargetLocation - AgentFragment.Position).GetSafeNormal();
                    FlowDirection = FMath::Lerp(FlowDirection, DirectPath, DirectPathWeight);
                }

                TargetFragment.DesiredDirection = FlowDirection.GetSafeNormal();
            }
        }
    });
}
