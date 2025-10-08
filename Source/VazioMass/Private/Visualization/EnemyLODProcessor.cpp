#include "Visualization/EnemyLODProcessor.h"

#include "MassProcessingPhaseManager.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "Visualization/EnemyLODFragment.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    // Distances are in Unreal units (centimeters). Keep enemies visible from gameplay ranges.
    constexpr float LOD0DistanceSqr = 1200.0f * 1200.0f;   // 12 meters
    constexpr float LOD1DistanceSqr = 2800.0f * 2800.0f;   // 28 meters
    constexpr float LOD2DistanceSqr = 5000.0f * 5000.0f;   // 50 meters
}

UEnemyLODProcessor::UEnemyLODProcessor()
{
    EntityQuery.RegisterWithProcessor(*this);
    ExecutionOrder.ExecuteInGroup = FName(TEXT("PostPhysics"));
    ExecutionOrder.ExecuteAfter.Add(FName(TEXT("Movement")));
    bRequiresGameThreadExecution = false;
    ProcessorRequirements.AddSubsystemRequirement<UMassEntitySubsystem>(EMassFragmentAccess::ReadOnly);
}

void UEnemyLODProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    (void)EntityManager;
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FEnemyLODLevelFragment>(EMassFragmentAccess::ReadWrite);
}
void UEnemyLODProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
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

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        return;
    }

    const FVector ViewLocation = PlayerPawn->GetActorLocation();

    EntityQuery.ForEachEntityChunk(Context, [ViewLocation](FMassExecutionContext& ChunkContext)
    {
        const TConstArrayView<FTransformFragment> TransformFragments = ChunkContext.GetFragmentView<FTransformFragment>();
        TArrayView<FEnemyLODLevelFragment> LOD = ChunkContext.GetMutableFragmentView<FEnemyLODLevelFragment>();

        for (int32 Index = 0; Index < ChunkContext.GetNumEntities(); ++Index)
        {
            const FVector Location = TransformFragments[Index].GetTransform().GetLocation();
            const float DistanceSqr = FVector::DistSquared(Location, ViewLocation);

            int8 DesiredLOD = 3;
            if (DistanceSqr <= LOD0DistanceSqr)
            {
                DesiredLOD = 0;
            }
            else if (DistanceSqr <= LOD1DistanceSqr)
            {
                DesiredLOD = 1;
            }
            else if (DistanceSqr <= LOD2DistanceSqr)
            {
                DesiredLOD = 2;
            }

            LOD[Index].Level = DesiredLOD;
        }
    });
}
