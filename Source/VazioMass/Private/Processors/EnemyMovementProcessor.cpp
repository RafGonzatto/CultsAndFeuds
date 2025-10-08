#include "Processors/EnemyMovementProcessor.h"

#include "Fragments/EnemyAgentFragment.h"
#include "Fragments/EnemyCombatFragment.h"
#include "Fragments/EnemyTargetFragment.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassEntityManager.h"
#include "MassProcessingPhaseManager.h"
#include "MassEntitySubsystem.h"
#include "Systems/EnemyMassSystem.h"
#include "Controllers/EnemyMassController.h"
#include "Services/EnemyMovementService.h"
#include "Trace/Trace.h"

UEnemyMovementProcessor::UEnemyMovementProcessor()
{
    EntityQuery.RegisterWithProcessor(*this);
    ExecutionOrder.ExecuteInGroup = FName(TEXT("Movement"));
    ExecutionOrder.ExecuteAfter.Add(FName(TEXT("Behavior")));
    bRequiresGameThreadExecution = false;
    ProcessorRequirements.AddSubsystemRequirement<UMassEntitySubsystem>(EMassFragmentAccess::ReadOnly);
}

void UEnemyMovementProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    (void)EntityManager;
    EntityQuery.AddRequirement<FEnemyAgentFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FEnemyTargetFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FEnemyCombatFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
}

void UEnemyMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    const UMassEntitySubsystem* EntitySubsystem = Context.GetSubsystem<UMassEntitySubsystem>();

    if (!MovementService && EntitySubsystem)
    {
        if (UWorld* World = EntitySubsystem->GetWorld())
        {
            if (UEnemyMassSystem* MassSystem = World->GetSubsystem<UEnemyMassSystem>())
            {
                if (UEnemyMassController* Controller = MassSystem->GetController())
                {
                    MovementService = Controller->GetMovementService();
                }
            }
        }

        if (!MovementService)
        {
            MovementService = NewObject<UEnemyMovementService>(this);
            if (MovementService)
            {
                MovementService->Initialize(MaxSteeringForce, 150.0f, 400.0f);
            }
        }
    }

    EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& ChunkContext)
    {
        TRACE_CPUPROFILER_EVENT_SCOPE(EnemyMovementProcessor_Chunk);
        const float DeltaTime = ChunkContext.GetDeltaTimeSeconds();

        const TArrayView<FEnemyAgentFragment> AgentFragments = ChunkContext.GetMutableFragmentView<FEnemyAgentFragment>();
        const TConstArrayView<FEnemyTargetFragment> TargetFragments = ChunkContext.GetFragmentView<FEnemyTargetFragment>();
        const TConstArrayView<FEnemyCombatFragment> CombatFragments = ChunkContext.GetFragmentView<FEnemyCombatFragment>();
    TArrayView<FTransformFragment> TransformFragments = ChunkContext.GetMutableFragmentView<FTransformFragment>();

        const TConstArrayView<FEnemyAgentFragment> ConstAgents = AgentFragments;

        for (int32 EntityIndex = 0; EntityIndex < ChunkContext.GetNumEntities(); ++EntityIndex)
        {
            FEnemyAgentFragment& AgentFragment = AgentFragments[EntityIndex];
            const FEnemyTargetFragment& TargetFragment = TargetFragments[EntityIndex];
            const FEnemyCombatFragment& CombatFragment = CombatFragments[EntityIndex];
            FTransformFragment& TransformFragment = TransformFragments[EntityIndex];

            MovementService->UpdateAgent(AgentFragment, TargetFragment, CombatFragment, ConstAgents, CombatFragments, EntityIndex, DeltaTime);

            TransformFragment.SetTransform(AgentFragment.CachedTransform);
        }
    });
}
