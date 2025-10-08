#include "Visualization/EnemyMassVisualizationProcessor.h"

#include "DrawDebugHelpers.h"
#include "Fragments/EnemyAgentFragment.h"
#include "Fragments/EnemyTargetFragment.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassProcessingPhaseManager.h"

TAutoConsoleVariable<int32> GMassEnemyDebug(
    TEXT("mass.enemy.DebugVis"),
    0,
    TEXT("Toggle enemy Mass visualization debug drawing."),
    ECVF_Default);

UEnemyMassVisualizationProcessor::UEnemyMassVisualizationProcessor()
{
    EntityQuery.RegisterWithProcessor(*this);
    ExecutionOrder.ExecuteInGroup = FName(TEXT("PostPhysics"));
    ExecutionOrder.ExecuteAfter.Add(FName(TEXT("Movement")));
    bRequiresGameThreadExecution = false;
    ProcessorRequirements.AddSubsystemRequirement<UMassEntitySubsystem>(EMassFragmentAccess::ReadOnly);
}

void UEnemyMassVisualizationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    (void)EntityManager;
    EntityQuery.AddRequirement<FEnemyAgentFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FEnemyTargetFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
}

void UEnemyMassVisualizationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    if (GMassEnemyDebug.GetValueOnAnyThread() == 0)
    {
        return;
    }

    const UMassEntitySubsystem* EntitySubsystem = Context.GetSubsystem<UMassEntitySubsystem>();
    if (!EntitySubsystem)
    {
        return;
    }

    UWorld* World = EntitySubsystem->GetWorld();
    if (!World)
    {
        return;
    }

    EntityQuery.ForEachEntityChunk(Context, [World](FMassExecutionContext& ChunkContext)
    {
        const TConstArrayView<FEnemyAgentFragment> Agents = ChunkContext.GetFragmentView<FEnemyAgentFragment>();
        const TConstArrayView<FEnemyTargetFragment> Targets = ChunkContext.GetFragmentView<FEnemyTargetFragment>();
        const TConstArrayView<FTransformFragment> Transforms = ChunkContext.GetFragmentView<FTransformFragment>();

        for (int32 Index = 0; Index < ChunkContext.GetNumEntities(); ++Index)
        {
            const FVector Position = Transforms[Index].GetTransform().GetLocation();
            const FVector VelocityDir = Agents[Index].Velocity.GetSafeNormal();
            const FVector TargetDirection = (Targets[Index].TargetLocation - Position).GetSafeNormal();

            DrawDebugDirectionalArrow(World, Position, Position + VelocityDir * 150.0f, 20.0f, FColor::Green, false, -1.0f, 0, 2.0f);
            DrawDebugDirectionalArrow(World, Position, Position + TargetDirection * 150.0f, 15.0f, FColor::Purple, false, -1.0f, 0, 1.5f);
            DrawDebugSphere(World, Position, 40.0f, 12, FColor::Cyan, false, -1.0f, 0, 1.5f);
        }
    });
}
