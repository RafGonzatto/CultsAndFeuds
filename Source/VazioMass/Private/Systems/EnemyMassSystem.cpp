#include "Systems/EnemyMassSystem.h"

#include "Controllers/EnemyMassController.h"
#include "MassEntitySubsystem.h"
#include "MassCommandBuffer.h"
#include "MassSettings.h"
#include "MassProcessingPhaseManager.h"
#include "Engine/World.h"
#include "Processors/EnemyMovementProcessor.h"
#include "Processors/EnemyPerceptionProcessor.h"
#include "Processors/EnemyAIProcessor.h"
#include "Processors/EnemyHealthProcessor.h"
#include "Processors/EnemyDamageProcessor.h"
#include "Processors/EnemyCombatProcessor.h"
#include "Visualization/EnemyLODProcessor.h"
#include "Visualization/EnemyMassVisualizationProcessor.h"
#include "Navigation/EnemyFlowFieldProcessor.h"
#include "Enemy/EnemyTypes.h"
#include "Logging/VazioLogFacade.h"

DEFINE_LOG_CATEGORY(LogEnemyMassSystem);

void UEnemyMassSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UWorld* World = GetWorld();
    if (!World)
    {
        LOG_MASS(Warn, TEXT("EnemyMassSystem initialized without a world context."));
        return;
    }

    EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    if (!EntitySubsystem.IsValid())
    {
        LOG_MASS(Warn, TEXT("MassEntitySubsystem not found. Ensure MassEntity plugin is enabled."));
        return;
    }

    SharedCommandBuffer = MakeShared<FMassCommandBuffer>();

    if (!MassController)
    {
        MassController = NewObject<UEnemyMassController>(this);
    }

    if (MassController)
    {
        MassController->Initialize(World, EntitySubsystem.Get(), SharedCommandBuffer);
    }

    RegisterCoreProcessors();

    LOG_MASS(Info, TEXT("[Mass] Initialized"));
}

void UEnemyMassSystem::Deinitialize()
{
    if (MassController)
    {
        MassController->StopGameThreadTicker();
    }

    SharedCommandBuffer.Reset();
    EntitySubsystem.Reset();
    MassController = nullptr;

    Super::Deinitialize();
}

void UEnemyMassSystem::RegisterCoreProcessors()
{
    if (!EntitySubsystem.IsValid())
    {
        return;
    }

    // UE 5.6: defer processor acquisition to runtime via Mass settings or ensure they are part of the processing graph via module startup
    // For bring-up, we don't hard-create processors here to avoid API mismatches; they will auto-register via the Mass processing pipeline
}

UWorld* UEnemyMassSystem::GetWorldChecked() const
{
    if (UWorld* World = GetWorld())
    {
        return World;
    }

    ensureMsgf(false, TEXT("EnemyMassSystem world access failed."));
    return nullptr;
}

// TODO(DEPRECATE): Damage event queue disabled during bring-up
void UEnemyMassSystem::ApplyRadialDamageMass(const FVector& Origin, float Radius, float Amount, bool bCritical)
{
    if (Amount <= 0.0f || Radius <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    FEnemyDamageEvent Evt;
    Evt.Origin = Origin;
    Evt.Radius = Radius;
    Evt.Amount = Amount;
    Evt.bCritical = bCritical;

    FScopeLock Lock(&DamageEventsLock);
    PendingDamageEvents.Add(Evt);
}

void UEnemyMassSystem::DequeuePendingDamage(TArray<FEnemyDamageEvent>& OutEvents)
{
    FScopeLock Lock(&DamageEventsLock);
    OutEvents = MoveTemp(PendingDamageEvents);
    PendingDamageEvents.Reset();
}