#include "Visualization/EnemyLODApplyProcessor.h"

#include "MassExecutionContext.h"
#include "MassCommonFragments.h"

#include "Visualization/EnemyLODFragment.h"
#include "Visualization/EnemyISMSubsystem.h"
#include "Visualization/EnemyMassVisualizationProcessor.h"

UEnemyLODApplyProcessor::UEnemyLODApplyProcessor()
{
    Query.RegisterWithProcessor(*this);
    ExecutionOrder.ExecuteInGroup = FName(TEXT("PostPhysics"));
    ExecutionOrder.ExecuteAfter.Add(FName(TEXT("Movement")));
    bRequiresGameThreadExecution = false;
}

void UEnemyLODApplyProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    (void)EntityManager;
    Query.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    Query.AddRequirement<FEnemyLODLevelFragment>(EMassFragmentAccess::ReadOnly);
}

void UEnemyLODApplyProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    UWorld* World = Context.GetWorld();
    if (!World)
    {
        return;
    }

    UEnemyISMSubsystem* ISM = World->GetSubsystem<UEnemyISMSubsystem>();
    if (!ISM)
    {
        return;
    }

    const bool bDebugDraw = GMassEnemyDebug.GetValueOnAnyThread() > 0;

    Query.ForEachEntityChunk(Context, [ISM, bDebugDraw](FMassExecutionContext& ChunkContext)
    {
        const TConstArrayView<FEnemyLODLevelFragment> LOD = ChunkContext.GetFragmentView<FEnemyLODLevelFragment>();
        for (int32 i = 0; i < ChunkContext.GetNumEntities(); ++i)
        {
            if (!ISM->ShouldRenderLOD(LOD[i].Level))
            {
                continue; // would hide/skip ISM instance here
            }

            if (bDebugDraw)
            {
                // Additional debug handled by visualization processor
            }
        }
    });
}
