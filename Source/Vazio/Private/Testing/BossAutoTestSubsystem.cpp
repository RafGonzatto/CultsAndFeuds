#include "Testing/BossAutoTestSubsystem.h"
#include "Enemy/EnemySpawnerSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/Events.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/InputSettings.h"
#include "Logging/VazioLogFacade.h"

void UBossAutoTestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // GATE: Only initialize in Battle_Main level
    if (UWorld* World = GetWorld())
    {
        FString MapName = World->GetMapName();
        if (!MapName.Contains(TEXT("Battle_Main")))
        {
            LOG_ENEMIES(Verbose, TEXT("BossAutoTestSubsystem skipping initialization (not in Battle_Main level: %s)"), *MapName);
            return;
        }
    }
    else
    {
        LOG_ENEMIES(Verbose, TEXT("BossAutoTestSubsystem skipping initialization (no world)"));
        return;
    }
    
    LOG_ENEMIES(Info, TEXT("BossAutoTestSubsystem initialized"));
    LOG_ENEMIES(Info, TEXT("Console Commands available:"));
    LOG_ENEMIES(Info, TEXT("  SpawnBurrower - Spawna Burrower Boss"));
    LOG_ENEMIES(Info, TEXT("  SpawnVoidQueen - Spawna Void Queen Boss"));
    LOG_ENEMIES(Info, TEXT("  SpawnFallenWarlord - Spawna Fallen Warlord Boss"));
    LOG_ENEMIES(Info, TEXT("  SpawnHybridDemon - Spawna Hybrid Demon Boss"));
    LOG_ENEMIES(Info, TEXT("  SpawnAllBosses - Spawna todos em sequencia"));
    LOG_ENEMIES(Info, TEXT("  StopBoss - Para boss atual"));
    LOG_ENEMIES(Info, TEXT("  StartAutoTest - Ativa modo teste automatico"));
    
    // Aguardar um frame para garantir que outros subsistemas estão prontos
    if (UWorld* World = GetWorld())
    {
        FTimerHandle InitTimer;
        World->GetTimerManager().SetTimer(InitTimer, [this]()
        {
            GetEnemySpawner();
            SetupInputBindings();
            
            LOG_ENEMIES(Info, TEXT("Boss testing system ready!"));
            LOG_ENEMIES(Info, TEXT("Keyboard shortcuts:"));
            LOG_ENEMIES(Info, TEXT("  1 - Burrower Boss"));
            LOG_ENEMIES(Info, TEXT("  2 - Void Queen Boss"));
            LOG_ENEMIES(Info, TEXT("  3 - Fallen Warlord Boss"));
            LOG_ENEMIES(Info, TEXT("  4 - Hybrid Demon Boss"));
            LOG_ENEMIES(Info, TEXT("  5 - All Bosses"));
            LOG_ENEMIES(Info, TEXT("  0 - Stop Boss"));
        }, 0.1f, false);
    }
}

void UBossAutoTestSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        if (AutoTestTimer.IsValid())
        {
            World->GetTimerManager().ClearTimer(AutoTestTimer);
        }
    }

    Super::Deinitialize();
}



void UBossAutoTestSubsystem::SpawnBurrower()
{
    GetEnemySpawner();
    if (EnemySpawner)
    {
        EnemySpawner->SpawnTestBoss(FName("BurrowerBoss"));
        LogBossTest("Spawning Burrower Boss via console");
    }
}

void UBossAutoTestSubsystem::SpawnVoidQueen()
{
    GetEnemySpawner();
    if (EnemySpawner)
    {
        EnemySpawner->SpawnTestBoss(FName("VoidQueenBoss"));
        LogBossTest("Spawning Void Queen Boss via console");
    }
}

void UBossAutoTestSubsystem::SpawnFallenWarlord()
{
    GetEnemySpawner();
    if (EnemySpawner)
    {
        EnemySpawner->SpawnTestBoss(FName("FallenWarlordBoss"));
        LogBossTest("Spawning Fallen Warlord Boss via console");
    }
}

void UBossAutoTestSubsystem::SpawnHybridDemon()
{
    GetEnemySpawner();
    if (EnemySpawner)
    {
        EnemySpawner->SpawnTestBoss(FName("HybridDemonBoss"));
        LogBossTest("Spawning Hybrid Demon Boss via console");
    }
}

void UBossAutoTestSubsystem::SpawnAllBosses()
{
    GetEnemySpawner();
    if (EnemySpawner)
    {
        EnemySpawner->SpawnAllBossesForTesting();
        LogBossTest("Starting boss sequence test via console");
    }
}

void UBossAutoTestSubsystem::StopBoss()
{
    GetEnemySpawner();
    if (EnemySpawner && EnemySpawner->IsBossEncounterActive())
    {
        if (ABossEnemy* ActiveBoss = EnemySpawner->GetActiveBoss())
        {
            ActiveBoss->Destroy();
            LogBossTest("Stopped active boss encounter via console");
        }
    }
    else
    {
        LogBossTest("No active boss to stop");
    }
}

void UBossAutoTestSubsystem::StartAutoTest()
{
    bAutoTestMode = true;
    LogBossTest("Auto test mode ENABLED - Use number keys 1-5, 0 to control bosses");
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("Boss Auto Test: 1-4=Individual Bosses, 5=All Bosses, 0=Stop"));
    }
}

void UBossAutoTestSubsystem::StopAutoTest()
{
    bAutoTestMode = false;
    LogBossTest("Auto test mode DISABLED");
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
            TEXT("Boss Auto Test: DISABLED"));
    }
}

void UBossAutoTestSubsystem::SetupInputBindings()
{
    // Note: O sistema de input será configurado via GameMode ou PlayerController
    // Por ora, implementamos através do sistema de console commands
    LogBossTest("Input system ready - use console commands or StartAutoTest for keyboard shortcuts");
}

void UBossAutoTestSubsystem::OnKey1Pressed()
{
        SpawnBurrower();
}

void UBossAutoTestSubsystem::OnKey2Pressed()
{
    
        SpawnVoidQueen();
    
}

void UBossAutoTestSubsystem::OnKey3Pressed()
{
        SpawnFallenWarlord();
    
}

void UBossAutoTestSubsystem::OnKey4Pressed()
{
 
        SpawnHybridDemon();
    
}

void UBossAutoTestSubsystem::OnKey5Pressed()
{

        SpawnAllBosses();
    
}

void UBossAutoTestSubsystem::OnKey0Pressed()
{

        StopBoss();
    
}

void UBossAutoTestSubsystem::GetEnemySpawner()
{
    if (!EnemySpawner && GetWorld())
    {
        EnemySpawner = GetWorld()->GetSubsystem<UEnemySpawnerSubsystem>();
        if (!EnemySpawner)
        {
            LOG_ENEMIES(Error, TEXT("Could not find EnemySpawnerSubsystem"));
        }
    }
}

void UBossAutoTestSubsystem::LogBossTest(const FString& Message)
{
    LOG_ENEMIES(Info, TEXT("%s"), *Message);
    
    // Também mostrar na tela se possível
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
            FString::Printf(TEXT("Boss Test: %s"), *Message));
    }
}