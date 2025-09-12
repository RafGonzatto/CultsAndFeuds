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
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Swarm/SwarmSpawnRules.h"
#include "Enemy/EnemyTypes.h"

void UEnemySpawnerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeEnemyClasses();
    SpawnRng.Initialize(FMath::Rand());
    
    UE_LOG(LogEnemySpawn, Log, TEXT("EnemySpawnerSubsystem initialized"));
}

void UEnemySpawnerSubsystem::Deinitialize()
{
    // Clear all scheduled timers
    for (FTimerHandle& Handle : ScheduledTimers)
    {
        if (Handle.IsValid())
        {
            GetWorld()->GetTimerManager().ClearTimer(Handle);
        }
    }
    ScheduledTimers.Empty();
    
    Super::Deinitialize();
}

void UEnemySpawnerSubsystem::StartTimeline(const USpawnTimeline* Timeline, int32 Seed)
{
    if (!Timeline)
    {
        UE_LOG(LogEnemySpawn, Error, TEXT("StartTimeline called with null timeline"));
        return;
    }
    
    if (Seed != 0)
    {
        SpawnRng.Initialize(Seed);
        UE_LOG(LogEnemySpawn, Log, TEXT("Using seed %d for spawn timeline"), Seed);
    }
    
    // Clear existing timers
    for (FTimerHandle& Handle : ScheduledTimers)
    {
        if (Handle.IsValid())
        {
            GetWorld()->GetTimerManager().ClearTimer(Handle);
        }
    }
    ScheduledTimers.Empty();
    
    // Schedule all events
    for (const FSpawnEvent& Event : Timeline->Events)
    {
        ScheduleEvent(Event);
    }
    
    UE_LOG(LogEnemySpawn, Log, TEXT("Started spawn timeline with %d events"), Timeline->Events.Num());
}

void UEnemySpawnerSubsystem::SpawnLinear(FName Type, int32 Count, const FEnemyInstanceModifiers& Mods)
{
    UE_LOG(LogEnemySpawn, Log, TEXT("SpawnLinear: %s x%d (big=%d, immovable=%d, dissolve=%.1fs)"), 
           *Type.ToString(), Count, Mods.bBig ? 1 : 0, Mods.bImmovable ? 1 : 0, Mods.DissolveSeconds);
    
    for (int32 i = 0; i < Count; i++)
    {
        FVector SpawnLocation;
        if (FindSpawnPointLinear(SpawnLocation))
        {
            FTransform SpawnTransform;
            SpawnTransform.SetLocation(SpawnLocation);
            
            AEnemyBase* NewEnemy = SpawnOne(Type, SpawnTransform, Mods);
            if (NewEnemy)
            {
                UE_LOG(LogEnemySpawn, VeryVerbose, TEXT("Spawned %s at %s"), *Type.ToString(), *SpawnLocation.ToCompactString());
            }
        }
        else
        {
            UE_LOG(LogEnemySpawn, Warning, TEXT("Failed to find spawn point for %s"), *Type.ToString());
        }
    }
}

void UEnemySpawnerSubsystem::SpawnCircleAroundPlayer(FName Type, int32 Count, const FEnemyInstanceModifiers& Mods, float Radius)
{
    UE_LOG(LogEnemySpawn, Log, TEXT("SpawnCircleAroundPlayer: %s x%d (radius=%.1f, big=%d, immovable=%d, dissolve=%.1fs)"), 
           *Type.ToString(), Count, Radius, Mods.bBig ? 1 : 0, Mods.bImmovable ? 1 : 0, Mods.DissolveSeconds);
    
    for (int32 i = 0; i < Count; i++)
    {
        FVector SpawnLocation;
        if (FindSpawnPointCircle(Radius, i, Count, SpawnLocation))
        {
            FTransform SpawnTransform;
            SpawnTransform.SetLocation(SpawnLocation);
            
            AEnemyBase* NewEnemy = SpawnOne(Type, SpawnTransform, Mods);
            if (NewEnemy)
            {
                UE_LOG(LogEnemySpawn, VeryVerbose, TEXT("Spawned %s at %s"), *Type.ToString(), *SpawnLocation.ToCompactString());
            }
        }
        else
        {
            UE_LOG(LogEnemySpawn, Warning, TEXT("Failed to find spawn point for %s in circle"), *Type.ToString());
        }
    }
}

AEnemyBase* UEnemySpawnerSubsystem::SpawnOne(FName Type, const FTransform& Transform, const FEnemyInstanceModifiers& Mods)
{
    if (!CurrentEnemyConfig)
    {
        UE_LOG(LogEnemySpawn, Error, TEXT("No EnemyConfig set for spawner subsystem"));
        return nullptr;
    }
    
    const FEnemyArchetype* Archetype = CurrentEnemyConfig->GetArchetype(Type);
    if (!Archetype)
    {
        UE_LOG(LogEnemySpawn, Error, TEXT("No archetype found for enemy type: %s"), *Type.ToString());
        return nullptr;
    }
    
    AEnemyBase* NewEnemy = CreateEnemyActor(Type, Transform);
    if (!NewEnemy)
    {
        UE_LOG(LogEnemySpawn, Error, TEXT("Failed to create actor for enemy type: %s"), *Type.ToString());
        return nullptr;
    }
    
    // Apply archetype and modifiers
    NewEnemy->ApplyArchetypeAndModifiers(*Archetype, Mods);
    
    // Special handling for SplitterSlime parents
    if (Type == TEXT("SplitterSlime") && Archetype->Death == EOnDeathBehavior::Split)
    {
        NewEnemy->bIsParent = true;
    }
    
    return NewEnemy;
}

void UEnemySpawnerSubsystem::SetEnemyConfig(UEnemyConfig* Config)
{
    CurrentEnemyConfig = Config;
    UE_LOG(LogEnemySpawn, Log, TEXT("EnemyConfig set with %d archetypes"), Config ? Config->Archetypes.Num() : 0);
}

void UEnemySpawnerSubsystem::ScheduleEvent(const FSpawnEvent& Event)
{
    // CRITICAL DEBUG: Force immediate execution for time=0
    if (Event.TimeSeconds <= 0.0f)
    {
        UE_LOG(LogEnemySpawn, Error, TEXT("🚨 IMMEDIATE EXECUTION! Event time=%.3f - executing NOW"), Event.TimeSeconds);
        ExecuteSpawnEvent(Event);
        return;
    }
    
    FTimerHandle EventTimer;
    FTimerDelegate EventDelegate;
    EventDelegate.BindLambda([this, Event]()
    {
        UE_LOG(LogEnemySpawn, Error, TEXT("🎯 TIMER FIRED! Executing spawn event at time %.1f"), Event.TimeSeconds);
        ExecuteSpawnEvent(Event);
    });
    
    UE_LOG(LogEnemySpawn, Error, TEXT("📅 ABOUT TO SCHEDULE timer for %.3f seconds"), Event.TimeSeconds);
    
    GetWorld()->GetTimerManager().SetTimer(
        EventTimer,
        EventDelegate,
        Event.TimeSeconds,
        false
    );
    
    ScheduledTimers.Add(EventTimer);
    UE_LOG(LogEnemySpawn, Error, TEXT("📅 SCHEDULED timer for %.3f seconds (Valid: %s, Active: %s)"), 
           Event.TimeSeconds, 
           EventTimer.IsValid() ? TEXT("YES") : TEXT("NO"),
           GetWorld()->GetTimerManager().IsTimerActive(EventTimer) ? TEXT("YES") : TEXT("NO"));
}

void UEnemySpawnerSubsystem::ExecuteSpawnEvent(const FSpawnEvent& Event)
{
    UE_LOG(LogEnemySpawn, Error, TEXT("🚀🚀🚀 EXECUTING SPAWN EVENT at time %.1f - Linear:%d Circles:%d"), 
           Event.TimeSeconds, Event.Linear.Num(), Event.Circles.Num());
    
    // Process linear spawns
    for (const FTypeCount& TypeCount : Event.Linear)
    {
        UE_LOG(LogEnemySpawn, Error, TEXT("📍📍📍 LINEAR SPAWN: %s x%d"), *TypeCount.Type.ToString(), TypeCount.Count);
        SpawnLinear(TypeCount.Type, TypeCount.Count, TypeCount.Mods);
    }
    
    // Process circle spawns
    for (const FCircleSpawn& CircleSpawn : Event.Circles)
    {
        SpawnCircleAroundPlayer(CircleSpawn.Type, CircleSpawn.Count, CircleSpawn.Mods, CircleSpawn.Radius);
    }
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
    
    // Use existing spawn rules from SwarmSpawnRules
    FVector CandidatePoint;
    if (SwarmSpawn::FindSpawnPointInView(*GetWorld(), PlayerLocation, PlayerRotation, 
                                         LinearSpawnMinDistance, LinearSpawnMaxDistance, 200.f, CandidatePoint))
    {
        // Project to navmesh or ground
        return SwarmSpawn::ProjectToNavmeshOrGround(*GetWorld(), CandidatePoint, true, OutLocation);
    }
    
    return false;
}

bool UEnemySpawnerSubsystem::FindSpawnPointCircle(float Radius, int32 Index, int32 TotalCount, FVector& OutLocation)
{
    APawn* PlayerPawn = GetPlayerPawn();
    if (!PlayerPawn)
    {
        return false;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Calculate angle for this spawn point
    float Angle = (2.0f * PI * Index) / TotalCount;
    FVector Offset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Radius;
    FVector CandidatePoint = PlayerLocation + Offset;
    
    // Project to navmesh or ground
    return SwarmSpawn::ProjectToNavmeshOrGround(*GetWorld(), CandidatePoint, true, OutLocation);
}

AEnemyBase* UEnemySpawnerSubsystem::CreateEnemyActor(FName Type, const FTransform& Transform)
{
    TSubclassOf<AEnemyBase>* EnemyClass = EnemyClasses.Find(Type);
    if (!EnemyClass || !*EnemyClass)
    {
        UE_LOG(LogEnemySpawn, Error, TEXT("No class registered for enemy type: %s"), *Type.ToString());
        return nullptr;
    }
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    return GetWorld()->SpawnActor<AEnemyBase>(*EnemyClass, Transform, SpawnParams);
}

void UEnemySpawnerSubsystem::InitializeEnemyClasses()
{
    EnemyClasses.Empty();
    
    // Register all enemy types
    EnemyClasses.Add(TEXT("NormalEnemy"), ANormalEnemy::StaticClass());
    EnemyClasses.Add(TEXT("HeavyEnemy"), AHeavyEnemy::StaticClass());
    EnemyClasses.Add(TEXT("RangedEnemy"), ARangedEnemy::StaticClass());
    EnemyClasses.Add(TEXT("DashEnemy"), ADashEnemy::StaticClass());
    EnemyClasses.Add(TEXT("AuraEnemy"), AAuraEnemy::StaticClass());
    EnemyClasses.Add(TEXT("SplitterSlime"), ASplitterSlime::StaticClass());
    EnemyClasses.Add(TEXT("GoldEnemy"), AGoldEnemy::StaticClass());
    
    UE_LOG(LogEnemySpawn, Log, TEXT("Registered %d enemy classes"), EnemyClasses.Num());
}

APawn* UEnemySpawnerSubsystem::GetPlayerPawn() const
{
    return UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}
