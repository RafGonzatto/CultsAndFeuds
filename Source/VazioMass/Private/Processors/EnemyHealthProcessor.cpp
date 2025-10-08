#include "Processors/EnemyHealthProcessor.h"

#include "Fragments/EnemyAgentFragment.h"
#include "Fragments/EnemyConfigSharedFragment.h"
#include "MassExecutionContext.h"
#include "MassProcessingPhaseManager.h"

UEnemyHealthProcessor::UEnemyHealthProcessor()
{
    EntityQuery.RegisterWithProcessor(*this);
    ExecutionOrder.ExecuteInGroup = FName(TEXT("PostPhysics"));
    ExecutionOrder.ExecuteAfter.Add(FName(TEXT("Behavior")));
    bRequiresGameThreadExecution = false;
}

void UEnemyHealthProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    (void)EntityManager;
    EntityQuery.AddRequirement<FEnemyAgentFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FEnemyConfigSharedFragment>(EMassFragmentAccess::ReadOnly);
}

void UEnemyHealthProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& ChunkContext)
    {
        const TConstArrayView<FEnemyConfigSharedFragment> ConfigFragments = ChunkContext.GetFragmentView<FEnemyConfigSharedFragment>();
        TArrayView<FEnemyAgentFragment> AgentFragments = ChunkContext.GetMutableFragmentView<FEnemyAgentFragment>();

        for (int32 Index = 0; Index < ChunkContext.GetNumEntities(); ++Index)
        {
            const float MaxHealth = FMath::Max(0.0f, ConfigFragments[Index].Health);
            FEnemyAgentFragment& AgentFragment = AgentFragments[Index];
            AgentFragment.Health = FMath::Clamp(AgentFragment.Health, 0.0f, MaxHealth);
        }
    });
}
