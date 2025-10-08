#include "Processors/EnemyCombatProcessor.h"

#include "Fragments/EnemyAgentFragment.h"
#include "Fragments/EnemyCombatFragment.h"
#include "Fragments/EnemyTargetFragment.h"
#include "Fragments/EnemyConfigSharedFragment.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassEntityView.h"
#include "Trace/Trace.h"
#include "DrawDebugHelpers.h"
#include "Async/Async.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"

#include "Logging/VazioLogFacade.h"

namespace
{
struct FCombatExecutionData
{
    FVector Location = FVector::ZeroVector;
    FVector TargetLocation = FVector::ZeroVector;
    float AttackRange = 0.0f;
    float Damage = 0.0f;
    EEnemyAbilityFlags AbilityFlags = EEnemyAbilityFlags::None;
    bool bDashActive = false;
};
}

UEnemyCombatProcessor::UEnemyCombatProcessor()
{
    EntityQuery.RegisterWithProcessor(*this);
    ExecutionOrder.ExecuteInGroup = FName(TEXT("PostPhysics"));
    ExecutionOrder.ExecuteAfter.Add(FName(TEXT("Movement")));
    bRequiresGameThreadExecution = false;
    ProcessorRequirements.AddSubsystemRequirement<UMassEntitySubsystem>(EMassFragmentAccess::ReadOnly);
}

void UEnemyCombatProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    (void)EntityManager;
    EntityQuery.AddRequirement<FEnemyAgentFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FEnemyTargetFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FEnemyCombatFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FEnemyConfigSharedFragment>(EMassFragmentAccess::ReadOnly);
}

void UEnemyCombatProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(EnemyCombatProcessor);

    const UMassEntitySubsystem* EntitySubsystem = Context.GetSubsystem<UMassEntitySubsystem>();
    UWorld* World = EntitySubsystem ? EntitySubsystem->GetWorld() : nullptr;

    TArray<FCombatExecutionData, TInlineAllocator<16>> PendingExecutions;

    EntityQuery.ForEachEntityChunk(Context, [this, &PendingExecutions](FMassExecutionContext& ChunkContext)
    {
    const TConstArrayView<FEnemyAgentFragment> AgentFragments = ChunkContext.GetFragmentView<FEnemyAgentFragment>();
    TArrayView<FEnemyTargetFragment> TargetFragments = ChunkContext.GetMutableFragmentView<FEnemyTargetFragment>();
    TArrayView<FEnemyCombatFragment> CombatFragments = ChunkContext.GetMutableFragmentView<FEnemyCombatFragment>();
    const TConstArrayView<FEnemyConfigSharedFragment> ConfigFragments = ChunkContext.GetFragmentView<FEnemyConfigSharedFragment>();

        const int32 NumEntities = ChunkContext.GetNumEntities();
        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            FEnemyCombatFragment& CombatFragment = CombatFragments[EntityIndex];
            FEnemyTargetFragment& TargetFragment = TargetFragments[EntityIndex];

            if (!CombatFragment.bRequestAttack)
            {
                TargetFragment.bWantsAttack = false;
                continue;
            }

            CombatFragment.bRequestAttack = false;
            TargetFragment.bWantsAttack = false;

            FCombatExecutionData& NewEvent = PendingExecutions.AddDefaulted_GetRef();
            NewEvent.Location = AgentFragments[EntityIndex].Position;
            NewEvent.TargetLocation = TargetFragments[EntityIndex].TargetLocation;
            NewEvent.AttackRange = CombatFragment.AttackRange;
            NewEvent.Damage = ConfigFragments[EntityIndex].Damage;
            NewEvent.AbilityFlags = CombatFragment.AbilityFlags;
            NewEvent.bDashActive = CombatFragment.bDashActive;
        }
    });

    if (PendingExecutions.Num() == 0 || !World)
    {
        return;
    }

    const ENetMode NetMode = World->GetNetMode();
    TWeakObjectPtr<UWorld> WeakWorld = World;

    TArray<FCombatExecutionData, TInlineAllocator<16>> EventsCopy = PendingExecutions;
    AsyncTask(ENamedThreads::GameThread, [WeakWorld, NetMode, Events = MoveTemp(EventsCopy)]() mutable
    {
        UWorld* GameThreadWorld = WeakWorld.Get();
        if (!GameThreadWorld)
        {
            return;
        }

        APawn* TargetPawn = UGameplayStatics::GetPlayerPawn(GameThreadWorld, 0);
        const FVector PlayerLocation = TargetPawn ? TargetPawn->GetActorLocation() : FVector::ZeroVector;
        const UCapsuleComponent* Capsule = TargetPawn ? TargetPawn->FindComponentByClass<UCapsuleComponent>() : nullptr;
        const float CapsuleRadius = Capsule ? Capsule->GetScaledCapsuleRadius() : 0.0f;

        for (const FCombatExecutionData& Event : Events)
        {
            FColor DebugColor = FColor::Red;
            if (EnumHasAnyFlags(Event.AbilityFlags, EEnemyAbilityFlags::Aura))
            {
                DebugColor = FColor::Cyan;
            }
            else if (EnumHasAnyFlags(Event.AbilityFlags, EEnemyAbilityFlags::Dash))
            {
                DebugColor = FColor::Purple;
            }
            else if (EnumHasAnyFlags(Event.AbilityFlags, EEnemyAbilityFlags::Ranged))
            {
                DebugColor = FColor::Orange;
            }

            DrawDebugSphere(GameThreadWorld, Event.Location, Event.AttackRange, 24, DebugColor, false, 0.35f, 0, 2.0f);

            if (EnumHasAnyFlags(Event.AbilityFlags, EEnemyAbilityFlags::Ranged))
            {
                const FVector Direction = (Event.TargetLocation - Event.Location).GetSafeNormal();
                DrawDebugLine(GameThreadWorld, Event.Location, Event.Location + Direction * Event.AttackRange, DebugColor, false, 0.35f, 0, 2.0f);
            }

            if (Event.bDashActive)
            {
                const FVector DashDirection = (Event.TargetLocation - Event.Location).GetSafeNormal();
                DrawDebugDirectionalArrow(GameThreadWorld, Event.Location, Event.Location + DashDirection * Event.AttackRange * 0.75f, 45.0f, DebugColor, false, 0.35f, 0, 4.0f);
            }

            if (NetMode == NM_Client || !TargetPawn || Event.Damage <= 0.0f)
            {
                continue;
            }

            const float EffectiveRange = Event.AttackRange + CapsuleRadius;
            if (EffectiveRange <= KINDA_SMALL_NUMBER)
            {
                continue;
            }

            const float DistSq = FVector::DistSquared(PlayerLocation, Event.Location);
            if (DistSq <= FMath::Square(EffectiveRange))
            {
                UGameplayStatics::ApplyDamage(TargetPawn, Event.Damage, nullptr, nullptr, nullptr);
                LOG_ENEMIES(Verbose, TEXT("[MASS] Enemy applied %.1f damage to player (range=%.1f)"), Event.Damage, Event.AttackRange);
            }
        }
    });
}
