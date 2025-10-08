#include "Processors/EnemyDamageProcessor.h"

#include "Controllers/EnemyMassController.h"
#include "Fragments/EnemyAgentFragment.h"
#include "Fragments/EnemyConfigSharedFragment.h"
#include "Fragments/EnemyVisualFragment.h"
#include "Visualization/EnemyISMSubsystem.h"
#include "MassCommandBuffer.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassProcessingPhaseManager.h"
#include "Systems/EnemyMassSystem.h"
#include "Engine/World.h"
#include "Async/Async.h"
#include "UI/DamageTextService.h"
#include "World/XPDropService.h"
#include "MassCommonFragments.h"
#include "MassEntityView.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    constexpr float CriticalDamageMultiplier = 1.5f;
}

UEnemyDamageProcessor::UEnemyDamageProcessor()
{
    EntityQuery.RegisterWithProcessor(*this);
    ExecutionOrder.ExecuteInGroup = FName(TEXT("PostPhysics"));
    ExecutionOrder.ExecuteAfter.Add(FName(TEXT("Movement")));
    bRequiresGameThreadExecution = false;
    ProcessorRequirements.AddSubsystemRequirement<UMassEntitySubsystem>(EMassFragmentAccess::ReadOnly);
}

void UEnemyDamageProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    (void)EntityManager;
    EntityQuery.AddRequirement<FEnemyAgentFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FEnemyConfigSharedFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FEnemyVisualHandleFragment>(EMassFragmentAccess::ReadWrite);
}

void UEnemyDamageProcessor::SetSharedCommandBuffer(const TSharedPtr<FMassCommandBuffer>& InCommandBuffer)
{
    CommandBuffer = InCommandBuffer;
}

void UEnemyDamageProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Apply pending radial damage events then remove dead entities and drop XP orbs
    const UMassEntitySubsystem* EntitySubsystem = Context.GetSubsystem<UMassEntitySubsystem>();
    UWorld* World = EntitySubsystem ? EntitySubsystem->GetWorld() : nullptr;

    // Gather pending damage
    TArray<FEnemyDamageEvent> DamageEvents;
    if (World)
    {
        if (UEnemyMassSystem* MassSystem = World->GetSubsystem<UEnemyMassSystem>())
        {
            MassSystem->DequeuePendingDamage(DamageEvents);
        }
    }

    TArray<FMassEntityHandle> ToRemove;
    TArray<int32> VisualsToRelease;

    EntityQuery.ForEachEntityChunk(Context, [this, &EntityManager, &ToRemove, World, &DamageEvents, &VisualsToRelease](FMassExecutionContext& ChunkContext)
    {
    const TConstArrayView<FEnemyConfigSharedFragment> ConfigFragments = ChunkContext.GetFragmentView<FEnemyConfigSharedFragment>();
    TArrayView<FEnemyAgentFragment> AgentFragments = ChunkContext.GetMutableFragmentView<FEnemyAgentFragment>();
    const TConstArrayView<FMassEntityHandle> Entities = ChunkContext.GetEntities();
    const TConstArrayView<FTransformFragment> TransformFragments = ChunkContext.GetFragmentView<FTransformFragment>();
    TArrayView<FEnemyVisualHandleFragment> VisualHandles = ChunkContext.GetMutableFragmentView<FEnemyVisualHandleFragment>();

        for (int32 Index = 0; Index < ChunkContext.GetNumEntities(); ++Index)
        {
            FEnemyAgentFragment& Agent = AgentFragments[Index];
            const FEnemyConfigSharedFragment& Cfg = ConfigFragments[Index];

            // Apply radial damages
            if (DamageEvents.Num() > 0)
            {
                const FVector Pos = TransformFragments[Index].GetTransform().GetLocation();
                float TotalDamage = 0.0f;
                bool bCritical = false;
                for (const FEnemyDamageEvent& Evt : DamageEvents)
                {
                    const float DistSq = FVector::DistSquared(Pos, Evt.Origin);
                    if (DistSq <= FMath::Square(Evt.Radius))
                    {
                        TotalDamage += Evt.Amount;
                        bCritical |= Evt.bCritical;
                    }
                }
                if (TotalDamage > 0.0f)
                {
                    Agent.Health = FMath::Clamp(Agent.Health - TotalDamage, 0.0f, Cfg.Health);
                    if (World)
                    {
                        if (UDamageTextService* DmgService = World->GetSubsystem<UDamageTextService>())
                        {
                            DmgService->ShowDamageNumber(TotalDamage, Pos, bCritical);
                        }
                    }
                }
            }

            // Clamp health in case other systems modify it
            Agent.Health = FMath::Clamp(Agent.Health, 0.0f, Cfg.Health);

            if (Agent.Health <= 0.0f)
            {
                if (World)
                {
                    const FVector SpawnLoc = TransformFragments[Index].GetTransform().GetLocation();
                    if (UXPDropService* XPService = World->GetSubsystem<UXPDropService>())
                    {
                        XPService->SpawnXPDrop(Cfg.XPReward, SpawnLoc);
                    }
                }

                ToRemove.Add(Entities[Index]);

                const int32 VisualHandle = VisualHandles[Index].VisualHandle;
                if (VisualHandle != INDEX_NONE)
                {
                    VisualsToRelease.Add(VisualHandle);
                    VisualHandles[Index].VisualHandle = INDEX_NONE;
                }
            }
        }
    });

    if (ToRemove.Num() > 0)
    {
        FMassCommandBuffer& DeferredCommands = Context.Defer();
        for (const FMassEntityHandle& E : ToRemove)
        {
            DeferredCommands.DestroyEntity(E);
        }
    }

    if (VisualsToRelease.Num() > 0 && World)
    {
        TWeakObjectPtr<UWorld> WeakWorld = World;
        TArray<int32> HandlesCopy = VisualsToRelease;
        AsyncTask(ENamedThreads::GameThread, [WeakWorld, Handles = MoveTemp(HandlesCopy)]()
        {
            if (UWorld* GameThreadWorld = WeakWorld.Get())
            {
                if (UEnemyISMSubsystem* VisualSubsystem = GameThreadWorld->GetSubsystem<UEnemyISMSubsystem>())
                {
                    for (const int32 Handle : Handles)
                    {
                        VisualSubsystem->ReleaseVisual(Handle);
                    }
                }
            }
        });
    }
}

void UEnemyDamageProcessor::ResolveServices(UMassEntitySubsystem* EntitySubsystem)
{
    // TODO(DEPRECATE): Service resolution disabled
}
