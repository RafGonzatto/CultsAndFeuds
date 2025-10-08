#include "Visualization/EnemyVisualSyncProcessor.h"

#include "Fragments/EnemyVisualFragment.h"
#include "Visualization/EnemyISMSubsystem.h"
#include "Visualization/EnemyLODFragment.h"

#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassEntitySubsystem.h"

UEnemyVisualSyncProcessor::UEnemyVisualSyncProcessor()
{
    EntityQuery.RegisterWithProcessor(*this);
    ExecutionOrder.ExecuteInGroup = FName(TEXT("PostPhysics"));
    ExecutionOrder.ExecuteAfter.Add(FName(TEXT("Movement")));
    bRequiresGameThreadExecution = true;
    ProcessorRequirements.AddSubsystemRequirement<UMassEntitySubsystem>(EMassFragmentAccess::ReadOnly);
}

void UEnemyVisualSyncProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    (void)EntityManager;
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FEnemyVisualHandleFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FEnemyLODLevelFragment>(EMassFragmentAccess::ReadOnly);
}

void UEnemyVisualSyncProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    UWorld* World = Context.GetWorld();
    if (!World)
    {
        return;
    }

    UEnemyISMSubsystem* VisualSubsystem = World->GetSubsystem<UEnemyISMSubsystem>();
    if (!VisualSubsystem)
    {
        return;
    }

    EntityQuery.ForEachEntityChunk(Context, [VisualSubsystem](FMassExecutionContext& ChunkContext)
    {
        const TConstArrayView<FTransformFragment> Transforms = ChunkContext.GetFragmentView<FTransformFragment>();
        const TConstArrayView<FEnemyVisualHandleFragment> VisualHandles = ChunkContext.GetFragmentView<FEnemyVisualHandleFragment>();
        const TConstArrayView<FEnemyLODLevelFragment> LODLevels = ChunkContext.GetFragmentView<FEnemyLODLevelFragment>();

        for (int32 Index = 0; Index < ChunkContext.GetNumEntities(); ++Index)
        {
            const int32 Handle = VisualHandles[Index].VisualHandle;
            if (Handle == INDEX_NONE)
            {
                continue;
            }

            const bool bVisible = VisualSubsystem->ShouldRenderLOD(LODLevels[Index].Level);
            VisualSubsystem->SetVisualActive(Handle, bVisible);
            if (!bVisible)
            {
                continue;
            }

            VisualSubsystem->UpdateVisual(Handle, Transforms[Index].GetTransform());
        }
    });
}
