#include "Processors/EnemyPerceptionProcessor.h"

#include "Fragments/EnemyAgentFragment.h"
#include "Fragments/EnemyTargetFragment.h"
#include "MassCommonFragments.h"
#include "MassProcessingPhaseManager.h"
#include "MassExecutionContext.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"
#include "Services/EnemyPerceptionService.h"
#include "Systems/EnemyMassSystem.h"
#include "Controllers/EnemyMassController.h"

UEnemyPerceptionProcessor::UEnemyPerceptionProcessor()
{
    EntityQuery.RegisterWithProcessor(*this);
    ExecutionOrder.ExecuteInGroup = FName(TEXT("SyncWorldToMass"));
    bRequiresGameThreadExecution = false;
    ProcessorRequirements.AddSubsystemRequirement<UMassEntitySubsystem>(EMassFragmentAccess::ReadOnly);
}

void UEnemyPerceptionProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    (void)EntityManager;
    EntityQuery.AddRequirement<FEnemyAgentFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FEnemyTargetFragment>(EMassFragmentAccess::ReadWrite);
}

void UEnemyPerceptionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
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

    const FVector TargetLocation = PerceptionService->GetPrimaryTargetLocation();

    EntityQuery.ForEachEntityChunk(Context, [TargetLocation](FMassExecutionContext& ChunkContext)
    {
        TArrayView<FEnemyTargetFragment> TargetFragments = ChunkContext.GetMutableFragmentView<FEnemyTargetFragment>();
        for (FEnemyTargetFragment& TargetFragment : TargetFragments)
        {
            TargetFragment.TargetLocation = TargetLocation;
        }
    });
}
