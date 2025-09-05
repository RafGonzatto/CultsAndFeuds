#include "Swarm/SwarmManager.h"
#include "Swarm/SwarmVisualComponent.h"
#include "Swarm/SwarmSpawnerComponent.h"
#include "SwarmProjectilePool.h" // corrected path
#include "Swarm/SwarmSubsystem.h"
#include "Swarm/SwarmVisibilityComponent.h"

#include "Components/SceneComponent.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"

ASwarmManager::ASwarmManager()
{
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    Visual = CreateDefaultSubobject<USwarmVisualComponent>(TEXT("Visual"));
    Spawner = CreateDefaultSubobject<USwarmSpawnerComponent>(TEXT("Spawner"));
    ProjPool = CreateDefaultSubobject<USwarmProjectilePool>(TEXT("ProjectilePool"));
    Visibility = CreateDefaultSubobject<USwarmVisibilityComponent>(TEXT("Visibility"));
}

void ASwarmManager::OnConstruction(const FTransform&)
{
    if (Visual && Config)
    {
        Visual->BuildFromConfig(*Config);
    }
}

void ASwarmManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogSwarm, Display, TEXT("SwarmManager BeginPlay: World=%s AutoStart=%d Config=%s"), *GetWorld()->GetName(), bAutoStart ? 1 : 0, Config ? *Config->GetName() : TEXT("<null>"));

    // Touch the subsystem unconditionally to force creation in this world
    if (UWorld* W = GetWorld())
    {
        USwarmSubsystem* SSProbe = W->GetSubsystem<USwarmSubsystem>();
        UE_LOG(LogSwarm, Display, TEXT("SwarmManager BeginPlay: Subsystem probe -> %s"), SSProbe ? TEXT("OK") : TEXT("NULL"));
    }

    if (bAutoStart)
    {
        if (!Config)
        {
            UE_LOG(LogSwarm, Warning, TEXT("SwarmManager: 'Config' is not set. Assign a USwarmConfig DataAsset (e.g., DA_SwarmConfig) in this actor to enable visuals and logic."));
        }
        else if (Config->EnemyTypes.Num() == 0)
        {
            UE_LOG(LogSwarm, Warning, TEXT("SwarmManager: Config '%s' has 0 EnemyTypes; nothing will spawn. Add at least one type under 'Types'."), *Config->GetName());
        }
        if (USwarmSubsystem* SS = GetWorld()->GetSubsystem<USwarmSubsystem>())
        {
            UE_LOG(LogSwarm, Warning, TEXT("INITCONFIG"));
            SS->InitWithConfig(Config, this);
        }
        else
        {
            UE_LOG(LogSwarm, Error, TEXT("SwarmManager: USwarmSubsystem is NULL in world %s"), *GetWorld()->GetName());
        }
    }

    if (ExitWidgetClass)
    {
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            ExitWidgetInstance = CreateWidget<UUserWidget>(PC, ExitWidgetClass);
            if (ExitWidgetInstance)
            {
                ExitWidgetInstance->AddToViewport(1000);
            }
        }
    }
}
