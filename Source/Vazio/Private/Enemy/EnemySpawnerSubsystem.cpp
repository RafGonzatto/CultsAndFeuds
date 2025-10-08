#include "Enemy/EnemySpawnerSubsystem.h"
#include "Enemy/EnemyConfig.h"
#include "Enemy/SpawnTimeline.h"
#include "Enemy/EnemyBase.h"
#include "Enemy/Types/NormalEnemy.h"
#include "Enemy/Types/HeavyEnemy.h"
#include "Enemy/Types/RangedEnemy.h"
#include "Enemy/Types/DashEnemy.h"
#include "Enemy/Types/AuraEnemy.h"
#include "Enemy/Types/SplitterSlime.h"
#include "Enemy/Types/GoldEnemy.h"
#include "Enemy/Types/BossEnemy.h"
#include "Enemy/Types/VoidQueenBoss.h"
#include "Enemy/Types/FallenWarlordBoss.h"
#include "Enemy/Types/BurrowerBoss.h"
#include "Enemy/Types/HybridDemonBoss.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Sound/SoundBase.h"
#include "Systems/EnemyMassSpawnerSubsystem.h"
#include "Systems/EnemyMassSystem.h"
#include "Systems/EnemyImplementationCVars.h"
#include "World/XPDropService.h"
#include "World/Common/Collectables/XPOrb.h"
#include "Logging/VazioLogFacade.h"

void UEnemySpawnerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    InitializeEnemyClasses();
    SpawnRng.Initialize(FMath::Rand());

    ActiveTimeline = nullptr;
    bBossEncounterActive = false;
    bRegularSpawnsPaused = false;
    DeferredEvents.Empty();

    if (UWorld* World = GetWorld())
    {
        World->GetSubsystem<UEnemyMassSystem>(); // ensure subsystem instantiation
        MassSpawner = World->GetSubsystem<UEnemyMassSpawnerSubsystem>();

        if (MassSpawner.IsValid())
        {
            LOG_ENEMIES(Info, TEXT("EnemySpawnerSubsystem connected to Mass spawner"));
            if (CurrentEnemyConfig)
            {
                MassSpawner->SetEnemyConfig(CurrentEnemyConfig.Get());
            }

            MassSpawner->OnBossSpawned.AddUObject(this, &UEnemySpawnerSubsystem::HandleMassBossSpawned);
            MassSpawner->OnBossHealthChanged.AddUObject(this, &UEnemySpawnerSubsystem::HandleMassBossHealthChanged);
            MassSpawner->OnBossDefeated.AddUObject(this, &UEnemySpawnerSubsystem::HandleMassBossDefeated);
        }
        else
        {
            LOG_ENEMIES(Warn, TEXT("EnemySpawnerSubsystem could not acquire Mass spawner; legacy path will remain disabled"));
        }

        if (UXPDropService* XPService = World->GetSubsystem<UXPDropService>())
        {
            XPService->XPDropActorClass = AXPOrb::StaticClass();
        }
    }

    LOG_ENEMIES(Info, TEXT("EnemySpawnerSubsystem initialized"));
}

void UEnemySpawnerSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        for (FTimerHandle& Handle : ScheduledTimers)
        {
            if (Handle.IsValid())
            {
                World->GetTimerManager().ClearTimer(Handle);
            }
        }

        for (FTimerHandle& Handle : ScheduledBossTimers)
        {
            if (Handle.IsValid())
            {
                World->GetTimerManager().ClearTimer(Handle);
            }
        }

        if (BossResumeHandle.IsValid())
        {
            World->GetTimerManager().ClearTimer(BossResumeHandle);
        }
    }

    ScheduledTimers.Empty();
    ScheduledBossTimers.Empty();
    DeferredEvents.Empty();

    if (MassSpawner.IsValid())
    {
        MassSpawner->OnBossSpawned.RemoveAll(this);
        MassSpawner->OnBossHealthChanged.RemoveAll(this);
        MassSpawner->OnBossDefeated.RemoveAll(this);
        MassSpawner.Reset();
    }

    ClearBossDelegates();
    ActiveBoss = nullptr;
    bBossEncounterActive = false;
    bRegularSpawnsPaused = false;
    ActiveMassBossHandle = FBossMassHandle();
    bMassBossEncounter = false;

    Super::Deinitialize();
}

void UEnemySpawnerSubsystem::StartTimeline(const USpawnTimeline* Timeline, int32 Seed)
{
    if (!Timeline)
    {
        LOG_ENEMIES(Error, TEXT("StartTimeline called with null timeline"));
        return;
    }

    ActiveTimeline = Timeline;

    if (Seed != 0)
    {
        SpawnRng.Initialize(Seed);
        LOG_ENEMIES(Debug, TEXT("Using deterministic seed %d for spawn timeline"), Seed);
    }

    if (UWorld* World = GetWorld())
    {
        for (FTimerHandle& Handle : ScheduledTimers)
        {
            if (Handle.IsValid())
            {
                World->GetTimerManager().ClearTimer(Handle);
            }
        }

        for (FTimerHandle& Handle : ScheduledBossTimers)
        {
            if (Handle.IsValid())
            {
                World->GetTimerManager().ClearTimer(Handle);
            }
        }

        if (BossResumeHandle.IsValid())
        {
            World->GetTimerManager().ClearTimer(BossResumeHandle);
        }
    }

    ScheduledTimers.Empty();
    ScheduledBossTimers.Empty();
    DeferredEvents.Empty();

    ClearBossDelegates();
    ActiveBoss = nullptr;
    bBossEncounterActive = false;
    bRegularSpawnsPaused = false;
    ActiveBossEntry = FBossSpawnEntry();
    ActiveMassBossHandle = FBossMassHandle();
    bMassBossEncounter = false;

    for (const FSpawnEvent& Event : Timeline->Events)
    {
        ScheduleEvent(Event);
    }

    for (const FBossSpawnEntry& BossEvent : Timeline->BossEvents)
    {
        ScheduleBossEvent(BossEvent);
    }

    LOG_ENEMIES(Info, TEXT("Started spawn timeline with %d events and %d boss events"), Timeline->Events.Num(), Timeline->BossEvents.Num());
}

void UEnemySpawnerSubsystem::SpawnLinear(FName Type, int32 Count, const FEnemyInstanceModifiers& Mods)
{
    LOG_ENEMIES(Debug, TEXT("SpawnLinear: %s x%d"), *Type.ToString(), Count);

    if (!ShouldUseMassSpawning())
    {
        LOG_ENEMIES(Warn, TEXT("TODO(DEPRECATE): Legacy actor-based SpawnLinear disabled; Mass spawner unavailable for %s"), *Type.ToString());
        return;
    }

    TArray<FTransform> SpawnTransforms;
    SpawnTransforms.Reserve(Count);

    for (int32 i = 0; i < Count; i++)
    {
        FVector SpawnLocation;
        if (FindSpawnPointLinear(SpawnLocation))
        {
            SpawnTransforms.Emplace(FQuat::Identity, SpawnLocation);
        }
        else
        {
            LOG_ENEMIES(Warn, TEXT("Failed to find spawn point for %s"), *Type.ToString());
        }
    }

    if (SpawnTransforms.Num() > 0)
    {
        MassSpawner->SpawnEnemiesAtTransforms(Type, SpawnTransforms, Mods);
    }
}

void UEnemySpawnerSubsystem::SpawnCircleAroundPlayer(FName Type, int32 Count, const FEnemyInstanceModifiers& Mods, float Radius)
{
    LOG_ENEMIES(Info, TEXT("SpawnCircleAroundPlayer: %s x%d (radius %.1f)"), *Type.ToString(), Count, Radius);

    if (!ShouldUseMassSpawning())
    {
        LOG_ENEMIES(Warn, TEXT("TODO(DEPRECATE): Legacy actor-based circular spawn disabled; Mass spawner unavailable for %s"), *Type.ToString());
        return;
    }

    TArray<FTransform> SpawnTransforms;
    SpawnTransforms.Reserve(Count);

    for (int32 i = 0; i < Count; i++)
    {
        FVector SpawnLocation;
        if (FindSpawnPointCircle(Radius, i, Count, SpawnLocation))
        {
            SpawnTransforms.Emplace(FQuat::Identity, SpawnLocation);
        }
        else
        {
            LOG_ENEMIES(Warn, TEXT("Failed to find circular spawn point for %s"), *Type.ToString());
        }
    }

    if (SpawnTransforms.Num() > 0)
    {
        MassSpawner->SpawnEnemiesAtTransforms(Type, SpawnTransforms, Mods);
    }
}

AEnemyBase* UEnemySpawnerSubsystem::SpawnOne(FName Type, const FTransform& Transform, const FEnemyInstanceModifiers& Mods)
{
    if (ShouldUseMassSpawning())
    {
        LOG_ENEMIES(Info, TEXT("Routing SpawnOne request to Mass system for %s"), *Type.ToString());

        TArray<FTransform> SingleSpawn;
        SingleSpawn.Add(Transform);
        MassSpawner->SpawnEnemiesAtTransforms(Type, SingleSpawn, Mods);
        return nullptr;
    }

#if !UE_BUILD_SHIPPING
    LOG_ENEMIES(Error, TEXT("Legacy actor SpawnOne used for %s when Mass implementation should be used"), *Type.ToString());
#endif

#if 0 // DEPRECATED: Legacy actor-based enemy implementation
    LOG_ENEMIES(Trace, TEXT("TODO(DEPRECATE): Using legacy actor SpawnOne for %s"), *Type.ToString());

    if (!CurrentEnemyConfig)
    {
        LOG_ENEMIES(Error, TEXT("No EnemyConfig set for spawner subsystem"));
        return nullptr;
    }

    const FEnemyArchetype* Archetype = CurrentEnemyConfig->GetArchetype(Type);
    if (!Archetype)
    {
        LOG_ENEMIES(Error, TEXT("No archetype found for enemy type: %s"), *Type.ToString());
        return nullptr;
    }

    AEnemyBase* NewEnemy = CreateEnemyActor(Type, Transform);
    if (!NewEnemy)
    {
        LOG_ENEMIES(Error, TEXT("Failed to create actor for enemy type: %s"), *Type.ToString());
        return nullptr;
    }

    NewEnemy->ApplyArchetypeAndModifiers(*Archetype, Mods);

    if (Type == TEXT("SplitterSlime") && Archetype->Death == EOnDeathBehavior::Split)
    {
        NewEnemy->bIsParent = true;
    }

    return NewEnemy;
#else
    return nullptr;
#endif
}

void UEnemySpawnerSubsystem::SetEnemyConfig(UEnemyConfig* Config)
{
    CurrentEnemyConfig = Config;
    LOG_ENEMIES(Info, TEXT("EnemyConfig set with %d archetypes"), Config ? Config->Archetypes.Num() : 0);

    if (MassSpawner.IsValid())
    {
        MassSpawner->SetEnemyConfig(Config);
    }
}

bool UEnemySpawnerSubsystem::ShouldUseMassSpawning() const
{
    return MassSpawner.IsValid() && GetEnemyImpl() == 1;
}

void UEnemySpawnerSubsystem::ScheduleEvent(const FSpawnEvent& Event)
{
    if (!GetWorld())
    {
        return;
    }

    if (Event.TimeSeconds <= 0.0f)
    {
        ExecuteSpawnEvent(Event);
        return;
    }

    FTimerHandle EventTimer;
    FTimerDelegate EventDelegate;
    EventDelegate.BindLambda([this, Event]()
    {
        ExecuteSpawnEvent(Event);
    });

    GetWorld()->GetTimerManager().SetTimer(EventTimer, EventDelegate, Event.TimeSeconds, false);
    ScheduledTimers.Add(EventTimer);
}

void UEnemySpawnerSubsystem::ExecuteSpawnEvent(const FSpawnEvent& Event)
{
    if ((bBossEncounterActive || bRegularSpawnsPaused) && !Event.bAllowDuringBossEncounter)
    {
        DeferredEvents.Add(Event);
        LOG_ENEMIES(Info, TEXT("Deferred spawn event at %.2fs due to active boss"), Event.TimeSeconds);
        return;
    }

    for (const FTypeCount& TypeCount : Event.Linear)
    {
        SpawnLinear(TypeCount.Type, TypeCount.Count, TypeCount.Mods);
    }

    for (const FCircleSpawn& CircleSpawn : Event.Circles)
    {
        SpawnCircleAroundPlayer(CircleSpawn.Type, CircleSpawn.Count, CircleSpawn.Mods, CircleSpawn.Radius);
    }
}

void UEnemySpawnerSubsystem::ScheduleBossEvent(const FBossSpawnEntry& BossEvent)
{
    if (!GetWorld())
    {
        return;
    }

    if (BossEvent.BossType.IsNone())
    {
        LOG_ENEMIES(Warn, TEXT("Ignoring boss event with empty type"));
        return;
    }

    if (BossEvent.WarningLeadTime > 0.f && BossEvent.TimeSeconds > 0.f)
    {
        const float WarningTime = FMath::Max(0.f, BossEvent.TimeSeconds - BossEvent.WarningLeadTime);
        FTimerHandle WarningHandle;
        FTimerDelegate WarningDelegate;
        WarningDelegate.BindUObject(this, &UEnemySpawnerSubsystem::TriggerBossWarning, BossEvent);
        GetWorld()->GetTimerManager().SetTimer(WarningHandle, WarningDelegate, WarningTime, false);
        ScheduledBossTimers.Add(WarningHandle);
    }

    FTimerHandle SpawnHandle;
    FTimerDelegate SpawnDelegate;
    SpawnDelegate.BindUObject(this, &UEnemySpawnerSubsystem::BeginBossEncounter, BossEvent);
    GetWorld()->GetTimerManager().SetTimer(SpawnHandle, SpawnDelegate, BossEvent.TimeSeconds, false);
    ScheduledBossTimers.Add(SpawnHandle);
}

void UEnemySpawnerSubsystem::TriggerBossWarning(FBossSpawnEntry BossEvent)
{
    if (BossEvent.BossType.IsNone())
    {
        return;
    }

    LOG_ENEMIES(Info, TEXT("Boss warning: %s incoming in %.1fs"), *BossEvent.BossType.ToString(), BossEvent.WarningLeadTime);

    OnBossWarning.Broadcast(BossEvent);

    if (BossEvent.WarningSound && GetWorld())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), BossEvent.WarningSound);
    }
}

void UEnemySpawnerSubsystem::BeginBossEncounter(FBossSpawnEntry BossEvent)
{
    if (BossEvent.BossType.IsNone())
    {
        return;
    }

    if (ShouldUseMassSpawning())
    {
        if (!MassSpawner.IsValid())
        {
            LOG_ENEMIES(Error, TEXT("Mass spawning selected but Mass spawner unavailable for boss %s"), *BossEvent.BossType.ToString());
            return;
        }

        if (bBossEncounterActive)
        {
            LOG_ENEMIES(Warn, TEXT("Attempted to start boss %s while another boss is active"), *BossEvent.BossType.ToString());
            return;
        }

        LOG_ENEMIES(Info, TEXT("Spawning boss %s using Mass system"), *BossEvent.BossType.ToString());

        const FTransform BossTransform = BuildBossSpawnTransform(BossEvent);
        TArray<FTransform> BossTransforms;
        BossTransforms.Add(BossTransform);

        ClearBossDelegates();
        ActiveBoss = nullptr;
        ActiveBossEntry = BossEvent;
        bMassBossEncounter = true;
        ActiveMassBossHandle = FBossMassHandle();

        const FBossMassHandle SpawnedHandle = MassSpawner->SpawnBossAtTransforms(BossEvent.BossType, BossTransforms, BossEvent.BossModifiers);
        if (!SpawnedHandle.IsValid())
        {
            LOG_ENEMIES(Error, TEXT("Failed to spawn boss %s using Mass system"), *BossEvent.BossType.ToString());
            ActiveBossEntry = FBossSpawnEntry();
            bMassBossEncounter = false;
            return;
        }

        ActiveMassBossHandle = SpawnedHandle;
        bBossEncounterActive = true;

        if (BossEvent.bPauseRegularSpawns)
        {
            PauseRegularSpawns();
        }

        return;
    }

#if 0 // DEPRECATED: Legacy actor-based boss implementation
    if (bBossEncounterActive)
    {
        LOG_ENEMIES(Warn, TEXT("Attempted to start boss %s while another boss is active"), *BossEvent.BossType.ToString());
        return;
    }

    const FTransform BossTransform = BuildBossSpawnTransform(BossEvent);
    ABossEnemy* SpawnedBoss = Cast<ABossEnemy>(SpawnOne(BossEvent.BossType, BossTransform, BossEvent.BossModifiers));

    if (!SpawnedBoss)
    {
        LOG_ENEMIES(Error, TEXT("Failed to spawn boss %s"), *BossEvent.BossType.ToString());
        return;
    }

    const FVector BossLocation = SpawnedBoss->GetActorLocation();
    LOG_ENEMIES(Warn, TEXT("[BossSpawn] Boss %s spawned at location: X=%.2f, Y=%.2f, Z=%.2f"), 
           *BossEvent.BossType.ToString(), BossLocation.X, BossLocation.Y, BossLocation.Z);

    LOG_ENEMIES(Info, TEXT("Boss %s has entered the battlefield"), *BossEvent.BossType.ToString());

    ClearBossDelegates();

    ActiveBoss = SpawnedBoss;
    ActiveBossEntry = BossEvent;
    bBossEncounterActive = true;

    if (BossEvent.bPauseRegularSpawns)
    {
        PauseRegularSpawns();
    }

    SpawnedBoss->OnBossHealthChanged.AddDynamic(this, &UEnemySpawnerSubsystem::HandleActiveBossHealthChanged);
    SpawnedBoss->OnBossDefeated.AddDynamic(this, &UEnemySpawnerSubsystem::HandleActiveBossDefeated);
    SpawnedBoss->OnBossPhaseChanged.AddDynamic(this, &UEnemySpawnerSubsystem::HandleActiveBossPhaseChanged);
    SpawnedBoss->OnBossTelegraph.AddDynamic(this, &UEnemySpawnerSubsystem::HandleActiveBossTelegraph);
    SpawnedBoss->OnBossAttackExecuted.AddDynamic(this, &UEnemySpawnerSubsystem::HandleActiveBossAttack);

    OnBossSpawned.Broadcast(SpawnedBoss, BossEvent);
    OnBossHealthChanged.Broadcast(SpawnedBoss->GetHealthFraction(), SpawnedBoss);
#else
    LOG_ENEMIES(Error, TEXT("Legacy actor-based boss spawning is disabled; boss %s will not spawn."), *BossEvent.BossType.ToString());
    return;
#endif
}

void UEnemySpawnerSubsystem::HandleActiveBossDefeated(ABossEnemy* Boss)
{
    LOG_ENEMIES(Info, TEXT("Boss encounter finished"));

    OnBossHealthChanged.Broadcast(0.f, Boss);
    OnBossEnded.Broadcast();

    ClearBossDelegates();

    ActiveBoss = nullptr;
    bBossEncounterActive = false;

    const float ResumeDelay = ActiveBossEntry.bPauseRegularSpawns ? (ActiveBossEntry.ResumeDelay + BossResumeDelayBuffer) : 0.f;
    ActiveBossEntry = FBossSpawnEntry();
    bMassBossEncounter = false;
    ActiveMassBossHandle = FBossMassHandle();

    ResumeRegularSpawns(ResumeDelay);
}

void UEnemySpawnerSubsystem::HandleActiveBossHealthChanged(float NormalizedHealth)
{
    ABossEnemy* BossPtr = ActiveBoss.Get();
    OnBossHealthChanged.Broadcast(NormalizedHealth, BossPtr);
}

void UEnemySpawnerSubsystem::HandleActiveBossPhaseChanged(int32 PhaseIndex, const FBossPhaseDefinition& PhaseData)
{
    OnBossPhaseChanged.Broadcast(PhaseIndex, PhaseData);
}

void UEnemySpawnerSubsystem::HandleActiveBossTelegraph(const FBossAttackPattern& Pattern)
{
    OnBossTelegraph.Broadcast(Pattern);
}

void UEnemySpawnerSubsystem::HandleActiveBossAttack(const FBossAttackPattern& Pattern)
{
    OnBossAttackExecuted.Broadcast(Pattern);
}

void UEnemySpawnerSubsystem::HandleMassBossSpawned(const FBossMassHandle& BossHandle, const FTransform& SpawnTransform)
{
    if (!bMassBossEncounter)
    {
        return;
    }

    if (!ActiveBossEntry.BossType.IsNone() && BossHandle.BossType != ActiveBossEntry.BossType)
    {
        return;
    }

    ActiveMassBossHandle = BossHandle;
    bBossEncounterActive = true;

    LOG_ENEMIES(Info, TEXT("Boss %s has entered the battlefield (Mass)"), *BossHandle.BossType.ToString());

    OnBossSpawned.Broadcast(nullptr, ActiveBossEntry);
    OnBossHealthChanged.Broadcast(1.f, nullptr);
}

void UEnemySpawnerSubsystem::HandleMassBossHealthChanged(const FBossMassHandle& BossHandle, float NormalizedHealth)
{
    if (!bMassBossEncounter || !ActiveMassBossHandle.IsValid())
    {
        return;
    }

    if (BossHandle.Entity != ActiveMassBossHandle.Entity)
    {
        return;
    }

    OnBossHealthChanged.Broadcast(NormalizedHealth, nullptr);
}

void UEnemySpawnerSubsystem::HandleMassBossDefeated(const FBossMassHandle& BossHandle)
{
    if (!bMassBossEncounter || !ActiveMassBossHandle.IsValid())
    {
        return;
    }

    if (BossHandle.Entity != ActiveMassBossHandle.Entity)
    {
        return;
    }

    LOG_ENEMIES(Info, TEXT("Boss encounter finished (Mass)"));

    OnBossHealthChanged.Broadcast(0.f, nullptr);
    OnBossEnded.Broadcast();

    ClearBossDelegates();
    ActiveBoss = nullptr;
    bBossEncounterActive = false;

    const float ResumeDelay = ActiveBossEntry.bPauseRegularSpawns ? (ActiveBossEntry.ResumeDelay + BossResumeDelayBuffer) : 0.f;
    ActiveBossEntry = FBossSpawnEntry();
    ActiveMassBossHandle = FBossMassHandle();
    bMassBossEncounter = false;

    ResumeRegularSpawns(ResumeDelay);
}

void UEnemySpawnerSubsystem::PauseRegularSpawns()
{
    bRegularSpawnsPaused = true;
}

void UEnemySpawnerSubsystem::ResumeRegularSpawns(float DelaySeconds)
{
    if (!GetWorld())
    {
        return;
    }

    if (DelaySeconds <= 0.f)
    {
        OnBossResumeTimerElapsed();
        return;
    }

    GetWorld()->GetTimerManager().SetTimer(BossResumeHandle, this, &UEnemySpawnerSubsystem::OnBossResumeTimerElapsed, DelaySeconds, false);
}

void UEnemySpawnerSubsystem::OnBossResumeTimerElapsed()
{
    bRegularSpawnsPaused = false;
    ResumeDeferredEvents();
}

void UEnemySpawnerSubsystem::ResumeDeferredEvents()
{
    if (DeferredEvents.Num() == 0)
    {
        return;
    }

    TArray<FSpawnEvent> Pending = DeferredEvents;
    DeferredEvents.Empty();

    for (const FSpawnEvent& Event : Pending)
    {
        ExecuteSpawnEvent(Event);
    }
}

FTransform UEnemySpawnerSubsystem::BuildBossSpawnTransform(const FBossSpawnEntry& BossEvent) const
{
    FVector SpawnLocation = FVector::ZeroVector;
    FRotator SpawnRotation = FRotator::ZeroRotator;

    if (APawn* PlayerPawn = GetPlayerPawn())
    {
        const FVector PlayerLocation = PlayerPawn->GetActorLocation();
        FVector Forward = PlayerPawn->GetActorForwardVector();

        if (BossEvent.bSpawnOutOfView)
        {
            Forward *= -1.f;
        }

        const float Distance = BossEvent.EntranceDistance > 0.f ? BossEvent.EntranceDistance : BossSpawnForwardDistance;
        SpawnLocation = PlayerLocation + Forward * Distance;
        SpawnLocation.Z = PlayerLocation.Z + BossSpawnHeightOffset;

        SpawnRotation = (PlayerLocation - SpawnLocation).Rotation();

        // Log da posição calculada para spawn
        LOG_ENEMIES(Warn, TEXT("[BossSpawnCalc] Calculated spawn position for boss: X=%.2f, Y=%.2f, Z=%.2f (Player at: X=%.2f, Y=%.2f, Z=%.2f, Distance: %.2f)"),
            SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z,
            PlayerLocation.X, PlayerLocation.Y, PlayerLocation.Z, Distance);
    }

    return FTransform(SpawnRotation, SpawnLocation);
}

bool UEnemySpawnerSubsystem::FindSpawnPointLinear(FVector& OutLocation)
{
    APawn* PlayerPawn = GetPlayerPawn();
    if (!PlayerPawn)
    {
        return false;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FRotator PlayerRotation = PlayerPawn->GetActorRotation();

    // TODO: SwarmSpawn namespace was removed - implement spawn point finding
    // For now, use simple circular spawn around player
    return FindSpawnPointCircle(LinearSpawnMinDistance, 0, 1, OutLocation);
}

bool UEnemySpawnerSubsystem::FindSpawnPointCircle(float Radius, int32 Index, int32 TotalCount, FVector& OutLocation)
{
    APawn* PlayerPawn = GetPlayerPawn();
    if (!PlayerPawn)
    {
        return false;
    }

    const FVector PlayerLocation = PlayerPawn->GetActorLocation();
    const float Angle = (2.0f * PI * Index) / FMath::Max(1, TotalCount);
    const FVector Offset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Radius;
    const FVector CandidatePoint = PlayerLocation + Offset;

    // Project point onto navigation mesh if available
    if (UWorld* World = GetWorld())
    {
        if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
        {
            FNavLocation ProjectedLocation;
            if (NavSys->ProjectPointToNavigation(CandidatePoint, ProjectedLocation))
            {
                OutLocation = ProjectedLocation.Location;
                return true;
            }
        }
    }

    // Fallback to direct point if nav projection fails
    OutLocation = CandidatePoint;
    OutLocation.Z = PlayerLocation.Z; // Keep same Z as player
    return true;
}

AEnemyBase* UEnemySpawnerSubsystem::CreateEnemyActor(FName Type, const FTransform& Transform)
{
    TSubclassOf<AEnemyBase>* EnemyClass = EnemyClasses.Find(Type);
    if (!EnemyClass || !*EnemyClass)
    {
        LOG_ENEMIES(Error, TEXT("No class registered for enemy type: %s"), *Type.ToString());
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AEnemyBase* SpawnedActor = GetWorld()->SpawnActor<AEnemyBase>(*EnemyClass, Transform, SpawnParams);
    
    if (SpawnedActor)
    {
        const FVector ActualSpawnLocation = SpawnedActor->GetActorLocation();
        LOG_ENEMIES(Warn, TEXT("[ActorSpawn] %s spawned successfully at actual location: X=%.2f, Y=%.2f, Z=%.2f"),
            *Type.ToString(), ActualSpawnLocation.X, ActualSpawnLocation.Y, ActualSpawnLocation.Z);
    }
    else
    {
        LOG_ENEMIES(Error, TEXT("[ActorSpawn] Failed to spawn %s"), *Type.ToString());
    }
    
    return SpawnedActor;
}

void UEnemySpawnerSubsystem::ClearBossDelegates()
{
    if (ABossEnemy* Boss = ActiveBoss.Get())
    {
        Boss->OnBossHealthChanged.RemoveDynamic(this, &UEnemySpawnerSubsystem::HandleActiveBossHealthChanged);
        Boss->OnBossDefeated.RemoveDynamic(this, &UEnemySpawnerSubsystem::HandleActiveBossDefeated);
        Boss->OnBossPhaseChanged.RemoveDynamic(this, &UEnemySpawnerSubsystem::HandleActiveBossPhaseChanged);
        Boss->OnBossTelegraph.RemoveDynamic(this, &UEnemySpawnerSubsystem::HandleActiveBossTelegraph);
        Boss->OnBossAttackExecuted.RemoveDynamic(this, &UEnemySpawnerSubsystem::HandleActiveBossAttack);
    }

    ActiveMassBossHandle = FBossMassHandle();
    bMassBossEncounter = false;
}

void UEnemySpawnerSubsystem::InitializeEnemyClasses()
{
    EnemyClasses.Empty();

    EnemyClasses.Add(TEXT("NormalEnemy"), ANormalEnemy::StaticClass());
    EnemyClasses.Add(TEXT("HeavyEnemy"), AHeavyEnemy::StaticClass());
    EnemyClasses.Add(TEXT("RangedEnemy"), ARangedEnemy::StaticClass());
    EnemyClasses.Add(TEXT("DashEnemy"), ADashEnemy::StaticClass());
    EnemyClasses.Add(TEXT("AuraEnemy"), AAuraEnemy::StaticClass());
    EnemyClasses.Add(TEXT("SplitterSlime"), ASplitterSlime::StaticClass());
    EnemyClasses.Add(TEXT("GoldEnemy"), AGoldEnemy::StaticClass());
    EnemyClasses.Add(TEXT("VoidQueenBoss"), AVoidQueenBoss::StaticClass());
    EnemyClasses.Add(TEXT("FallenWarlordBoss"), AFallenWarlordBoss::StaticClass());
    EnemyClasses.Add(TEXT("BurrowerBoss"), ABurrowerBoss::StaticClass());
    EnemyClasses.Add(TEXT("HybridDemonBoss"), AHybridDemonBoss::StaticClass());

    LOG_ENEMIES(Info, TEXT("Registered %d enemy classes"), EnemyClasses.Num());
}

APawn* UEnemySpawnerSubsystem::GetPlayerPawn() const
{
    return UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

void UEnemySpawnerSubsystem::SpawnTestBoss(FName BossType)
{
    if (BossType.IsNone())
    {
        LOG_ENEMIES(Warn, TEXT("SpawnTestBoss: BossType is None"));
        return;
    }

    if (bBossEncounterActive)
    {
        LOG_ENEMIES(Warn, TEXT("SpawnTestBoss: Boss encounter already active"));
        return;
    }

    // Criar entry de boss para teste
    FBossSpawnEntry TestBossEntry;
    TestBossEntry.BossType = BossType;
    TestBossEntry.TimeSeconds = 0.f;
    TestBossEntry.WarningLeadTime = 0.f;
    TestBossEntry.WarningDuration = 0.f;
    TestBossEntry.bPauseRegularSpawns = true;
    TestBossEntry.ResumeDelay = 2.f;
    TestBossEntry.Announcement = FText::FromString(FString::Printf(TEXT("Teste: %s spawnou!"), *BossType.ToString()));

    LOG_ENEMIES(Info, TEXT("Spawning test boss: %s"), *BossType.ToString());
    BeginBossEncounter(TestBossEntry);
}

void UEnemySpawnerSubsystem::SpawnAllBossesForTesting()
{
    if (bBossEncounterActive)
    {
        LOG_ENEMIES(Warn, TEXT("SpawnAllBossesForTesting: Boss encounter already active"));
        return;
    }

    LOG_ENEMIES(Info, TEXT("Starting boss testing sequence"));

    // Limpar timers antigos
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (FTimerHandle& Handle : ScheduledBossTimers)
    {
        if (Handle.IsValid())
        {
            World->GetTimerManager().ClearTimer(Handle);
        }
    }
    ScheduledBossTimers.Empty();

    // Array com todos os bosses para testar
    TArray<FName> BossesToTest = {
        TEXT("BurrowerBoss"),
        TEXT("VoidQueenBoss"),
        TEXT("FallenWarlordBoss"),
        TEXT("HybridDemonBoss")
    };

    // Spawnar cada boss em intervalos de 30 segundos
    float SpawnDelay = 5.f; // Começar em 5 segundos
    for (const FName& BossType : BossesToTest)
    {
        FBossSpawnEntry TestBoss;
        TestBoss.BossType = BossType;
        TestBoss.TimeSeconds = SpawnDelay;
        TestBoss.WarningLeadTime = 2.f;
        TestBoss.WarningDuration = 2.f;
        TestBoss.bPauseRegularSpawns = true;
        TestBoss.ResumeDelay = 3.f;
        TestBoss.Announcement = FText::FromString(FString::Printf(TEXT("Teste: %s está chegando!"), *BossType.ToString()));

        ScheduleBossEvent(TestBoss);
        SpawnDelay += 35.f; // 35 segundos entre cada boss
    }

    LOG_ENEMIES(Info, TEXT("Scheduled %d bosses for testing"), BossesToTest.Num());
}
