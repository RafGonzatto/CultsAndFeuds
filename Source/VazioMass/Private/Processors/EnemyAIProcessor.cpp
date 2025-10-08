#include "Processors/EnemyAIProcessor.h"

#include "Fragments/EnemyAgentFragment.h"
#include "Fragments/EnemyCombatFragment.h"
#include "Fragments/EnemyTargetFragment.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"
#include "MassProcessingPhaseManager.h"
#include "Services/EnemyAIService.h"
#include "Services/EnemyPerceptionService.h"
#include "Systems/EnemyMassSystem.h"
#include "Controllers/EnemyMassController.h"

UEnemyAIProcessor::UEnemyAIProcessor()
{
    EntityQuery.RegisterWithProcessor(*this);
    ExecutionOrder.ExecuteInGroup = FName(TEXT("Behavior"));
    bRequiresGameThreadExecution = false;
    ProcessorRequirements.AddSubsystemRequirement<UMassEntitySubsystem>(EMassFragmentAccess::ReadOnly);
}

void UEnemyAIProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    (void)EntityManager;
    EntityQuery.AddRequirement<FEnemyAgentFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FEnemyTargetFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FEnemyCombatFragment>(EMassFragmentAccess::ReadWrite);
}

void UEnemyAIProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    UEnemyPerceptionService* LocalPerceptionService = nullptr;

    const UMassEntitySubsystem* EntitySubsystem = Context.GetSubsystem<UMassEntitySubsystem>();

    if (EntitySubsystem)
    {
        if (UWorld* World = EntitySubsystem->GetWorld())
        {
            if (UEnemyMassSystem* MassSystem = World->GetSubsystem<UEnemyMassSystem>())
            {
                if (UEnemyMassController* Controller = MassSystem->GetController())
                {
                    if (!AIService)
                    {
                        AIService = Controller->GetAIService();
                    }
                    LocalPerceptionService = Controller->GetPerceptionService();
                }
            }
        }
    }

    if (!AIService)
    {
        return;
    }

    const FVector PrimaryTargetLocation = (LocalPerceptionService && LocalPerceptionService->HasValidPrimaryTarget())
        ? LocalPerceptionService->GetPrimaryTargetLocation()
        : FVector::ZeroVector;

    EntityQuery.ForEachEntityChunk(Context, [this, PrimaryTargetLocation](FMassExecutionContext& ChunkContext)
    {
        const float DeltaTime = ChunkContext.GetDeltaTimeSeconds();
        TArrayView<FEnemyAgentFragment> AgentFragments = ChunkContext.GetMutableFragmentView<FEnemyAgentFragment>();
        TArrayView<FEnemyTargetFragment> TargetFragments = ChunkContext.GetMutableFragmentView<FEnemyTargetFragment>();
        TArrayView<FEnemyCombatFragment> CombatFragments = ChunkContext.GetMutableFragmentView<FEnemyCombatFragment>();

        for (int32 EntityIndex = 0; EntityIndex < ChunkContext.GetNumEntities(); ++EntityIndex)
        {
            FEnemyAgentFragment& AgentFragment = AgentFragments[EntityIndex];
            FEnemyTargetFragment& TargetFragment = TargetFragments[EntityIndex];
            FEnemyCombatFragment& CombatFragment = CombatFragments[EntityIndex];
            AIService->EvaluateAgent(PrimaryTargetLocation, AgentFragment, TargetFragment, CombatFragment, DeltaTime);
        }
    });
}
