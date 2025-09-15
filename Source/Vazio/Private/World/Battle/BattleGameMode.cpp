#include "World/Battle/BattleGameMode.h"
#include "World/Common/Player/MyCharacter.h"
#include "World/Common/Player/MyPlayerController.h"

#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"

#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "NavigationSystem.h"

// Enemy System Integration
#include "Enemy/EnemySpawnerSubsystem.h"
#include "Enemy/EnemyConfig.h"
#include "Enemy/EnemySpawnHelper.h"

// Old Swarm System (for disabling)
#include "Swarm/SwarmSubsystem.h"

ABattleGameMode::ABattleGameMode()
{
    DefaultPawnClass = AMyCharacter::StaticClass();
    PlayerControllerClass = AMyPlayerController::StaticClass();

    // Prefer BP_MyCharacter if it exists so visuals/camera match City
    static ConstructorHelpers::FClassFinder<APawn> BPCharClass(TEXT("/Game/Characters/PlayerChar/BP_MyCharacter"));
    if (BPCharClass.Succeeded())
    {
        DefaultPawnClass = BPCharClass.Class;
    }

    // LEGAL: resolve Engine assets only here (constructor)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        CachedCubeMesh = CubeMeshAsset.Object;
    }
    static ConstructorHelpers::FObjectFinder<UMaterial> BasicMaterialAsset(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    if (BasicMaterialAsset.Succeeded())
    {
        CachedBasicMaterial = BasicMaterialAsset.Object;
    }
}

void ABattleGameMode::BeginPlay()
{
    Super::BeginPlay();
    CreatePlayerStartIfNeeded();

    // Create minimal visible environment so the level isn't black
    CreateBattleGround();
    CreateBasicLighting();

    // Trigger NavMesh rebuild so spawned ground contributes immediately
    if (UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
    {
        Nav->Build();
        UE_LOG(LogTemp, Display, TEXT("[BattleGM] Requested NavMesh rebuild"));
    }

    // Initialize new enemy system and disable old swarm system
    InitializeEnemySystem();
    DisableOldSwarmSystem();
    
    // Auto-start the first wave after a small delay to ensure everything is initialized
    FTimerHandle AutoStartTimer;
    GetWorldTimerManager().SetTimer(
        AutoStartTimer,
        this,
        &ABattleGameMode::StartTestWave,
        2.0f, // 2 second delay
        false
    );
    UE_LOG(LogTemp, Warning, TEXT("[BattleGM] Auto-starting first wave in 2 seconds..."));
}

void ABattleGameMode::CreatePlayerStartIfNeeded()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (TActorIterator<APlayerStart> It(World); It; ++It)
    {
        UE_LOG(LogTemp, Verbose, TEXT("[BattleGM] PlayerStart already present: %s"), *It->GetName());
        return; // already present
    }

    // Spawn a basic PlayerStart near the origin
    if (APlayerStart* PS = World->SpawnActor<APlayerStart>(FVector(0, 0, 150), FRotator::ZeroRotator))
    {
#if WITH_EDITOR
        PS->SetActorLabel(TEXT("BattlePlayerStart"));
#endif
        UE_LOG(LogTemp, Display, TEXT("[BattleGM] PlayerStart created at (0,0,150)"));
    }
}

AActor* ABattleGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<APlayerStart> It(World); It; ++It)
        {
            UE_LOG(LogTemp, Verbose, TEXT("[BattleGM] ChoosePlayerStart -> %s"), *It->GetName());
            return *It;
        }
    }

    // If none exists yet, create one and try again so the player always spawns
    CreatePlayerStartIfNeeded();
    if (World)
    {
        for (TActorIterator<APlayerStart> It(World); It; ++It)
        {
            UE_LOG(LogTemp, Verbose, TEXT("[BattleGM] ChoosePlayerStart (after create) -> %s"), *It->GetName());
            return *It;
        }
    }
    return Super::ChoosePlayerStart_Implementation(Player);
}

void ABattleGameMode::CreateBattleGround()
{
    UWorld* World = GetWorld();
    if (!World) return;
    // Reuse authored ground if present
    for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
    {
        if (It->GetName().Contains(TEXT("BattleGround")) || It->ActorHasTag(TEXT("BattleGround")))
        {
            UE_LOG(LogTemp, Verbose, TEXT("[BattleGM] Existing BattleGround detected: %s"), *It->GetName());
            return;
        }
    }
    // Use assets cached in constructor; do NOT use FObjectFinder here
    if (!CachedCubeMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BattleGM] CachedCubeMesh is null; skipping ground creation"));
        return;
    }

    AStaticMeshActor* Ground = World->SpawnActor<AStaticMeshActor>(FVector::ZeroVector, FRotator::ZeroRotator);
    if (!Ground) return;

    UStaticMeshComponent* Mesh = Ground->GetStaticMeshComponent();
    Mesh->SetMobility(EComponentMobility::Movable);
    Mesh->SetStaticMesh(CachedCubeMesh);
    // Enable collision so ground traces for spawn/grounding succeed
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Mesh->SetCollisionResponseToAllChannels(ECR_Block);
    Mesh->SetCanEverAffectNavigation(true);
    Ground->SetActorScale3D(FVector(100.f, 100.f, 1.0f)); // 100x100m, 1m thick
    Ground->Tags.Add(TEXT("BattleGround"));
    if (CachedBasicMaterial)
    {
        UMaterialInstanceDynamic* Dyn = UMaterialInstanceDynamic::Create(CachedBasicMaterial, Ground);
        if (Dyn)
        {
            Dyn->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.15f, 0.15f, 0.15f, 1.f));
            Mesh->SetMaterial(0, Dyn);
        }
    }
#if WITH_EDITOR
    Ground->SetActorLabel(TEXT("BattleGround"));
#endif
    UE_LOG(LogTemp, Display, TEXT("[BattleGM] Battle ground created"));
}

void ABattleGameMode::CreateBasicLighting()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Directional light (sun)
    // If any directional light already exists in the map, keep it
    for (TActorIterator<ADirectionalLight> It(World); It; ++It)
    {
        UE_LOG(LogTemp, Verbose, TEXT("[BattleGM] Using existing DirectionalLight: %s"), *It->GetName());
        goto AfterDirectional;
    }
    if (ADirectionalLight* Dir = World->SpawnActor<ADirectionalLight>(FVector(0, 0, 1000), FRotator(-45, 45, 0)))
    {
        if (UDirectionalLightComponent* C = Cast<UDirectionalLightComponent>(Dir->GetLightComponent()))
        {
            C->SetIntensity(6.f);
            C->SetLightColor(FLinearColor(1.0f, 0.95f, 0.85f));
        }
    }
AfterDirectional:

    // Ambient skylight
    for (TActorIterator<ASkyLight> It(World); It; ++It)
    {
        UE_LOG(LogTemp, Verbose, TEXT("[BattleGM] Using existing SkyLight: %s"), *It->GetName());
        return;
    }
    if (ASkyLight* Sky = World->SpawnActor<ASkyLight>(FVector(0, 0, 800), FRotator::ZeroRotator))
    {
        if (USkyLightComponent* C = Sky->GetLightComponent())
        {
            C->SetIntensity(2.5f);
            C->SetLightColor(FLinearColor(0.6f, 0.7f, 1.0f));
        }
    }
}

void ABattleGameMode::InitializeEnemySystem()
{
    UEnemySpawnerSubsystem* SpawnerSubsystem = GetWorld()->GetSubsystem<UEnemySpawnerSubsystem>();
    if (!SpawnerSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("[BattleGM] Could not get EnemySpawnerSubsystem"));
        return;
    }

    // Create or use existing enemy config
    if (!DefaultEnemyConfig)
    {
        DefaultEnemyConfig = UEnemyConfig::CreateDefaultConfig();
        UE_LOG(LogTemp, Log, TEXT("[BattleGM] Created default enemy config"));
    }

    SpawnerSubsystem->SetEnemyConfig(DefaultEnemyConfig);
    UE_LOG(LogTemp, Warning, TEXT("[BattleGM] Enemy system initialized - old swarm system disabled"));
}

void ABattleGameMode::DisableOldSwarmSystem()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // AGGRESSIVE SHUTDOWN: Find and destroy SwarmManager actors
    TArray<AActor*> ActorsToDestroy;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (Actor && (Actor->GetClass()->GetName().Contains(TEXT("SwarmManager")) || 
                      Actor->GetClass()->GetName().Contains(TEXT("BP_SwarmManager"))))
        {
            ActorsToDestroy.Add(Actor);
            UE_LOG(LogTemp, Warning, TEXT("[BattleGM] Marked SwarmManager for destruction: %s"), *Actor->GetName());
        }
    }
    
    // Destroy all SwarmManager actors
    for (AActor* Actor : ActorsToDestroy)
    {
        Actor->SetActorTickEnabled(false);
        Actor->SetActorHiddenInGame(true);
        Actor->Destroy();
        UE_LOG(LogTemp, Warning, TEXT("[BattleGM] DESTROYED SwarmManager: %s"), *Actor->GetName());
    }
    
    // Also disable SwarmSubsystem directly (without deinitializing engine-managed subsystem)
    if (USwarmSubsystem* SwarmSS = World->GetSubsystem<USwarmSubsystem>())
    {
        SwarmSS->Disable();
        UE_LOG(LogTemp, Warning, TEXT("[BattleGM] DISABLED SwarmSubsystem (soft)"));
    }
}

void ABattleGameMode::StartEnemyWave(const FString& JSONWaveData, int32 Seed)
{
    UEnemySpawnHelper::QuickSpawnEnemies(this, JSONWaveData, Seed);
    UE_LOG(LogTemp, Warning, TEXT("[BattleGM] Started enemy wave with seed %d"), Seed);
}

void ABattleGameMode::StartDefaultWave()
{
    FString DefaultWaveJSON = UEnemySpawnHelper::GetExampleJSON();
    StartEnemyWave(DefaultWaveJSON, FMath::Rand());
}

void ABattleGameMode::StartTestWave()
{
    // CORRECT JSON FORMAT for SpawnTimeline system
    FString TestWaveJSON = TEXT(R"({
        "spawnEvents": [
            {
                "time": 0.0,
                "spawns": {
                    "NormalEnemy": {
                        "count": 3,
                        "big": true
                    },
                    "circle": [
                        {
                            "type": "HeavyEnemy",
                            "count": 2,
                            "radius": 400
                        }
                    ]
                }
            },
            {
                "time": 5.0,
                "spawns": {
                    "RangedEnemy": 5,
                    "DashEnemy": {
                        "count": 2,
                        "big": false
                    }
                }
            }
        ]
    })");

    StartEnemyWave(TestWaveJSON, FMath::Rand());
    UE_LOG(LogTemp, Warning, TEXT("[BattleGM] Started test wave from JSON"));
}
