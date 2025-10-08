#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Enemy/EnemyTypes.h"
#include "Enemy/Types/BossEnemy.h"
#include "Systems/EnemyMassSpawnerSubsystem.h"
#include "EnemySpawnerSubsystem.generated.h"

class UEnemyConfig;
class USpawnTimeline;
class AEnemyBase;
class UEnemyMassSpawnerSubsystem;

DECLARE_MULTICAST_DELEGATE_OneParam(FSpawnerBossWarningSignature, const FBossSpawnEntry&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FSpawnerBossSpawnSignature, ABossEnemy*, const FBossSpawnEntry&);
DECLARE_MULTICAST_DELEGATE(FSpawnerBossEndSignature);
DECLARE_MULTICAST_DELEGATE_TwoParams(FSpawnerBossHealthSignature, float, ABossEnemy*);
DECLARE_MULTICAST_DELEGATE_TwoParams(FSpawnerBossPhaseSignature, int32, const FBossPhaseDefinition&);
DECLARE_MULTICAST_DELEGATE_OneParam(FSpawnerBossTelegraphSignature, const FBossAttackPattern&);
DECLARE_MULTICAST_DELEGATE_OneParam(FSpawnerBossAttackSignature, const FBossAttackPattern&);

UCLASS()
class VAZIO_API UEnemySpawnerSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Enemy Spawner")
    void StartTimeline(const USpawnTimeline* Timeline, int32 Seed = 0);

    UFUNCTION(BlueprintCallable, Category = "Enemy Spawner")
    void SpawnLinear(FName Type, int32 Count, const FEnemyInstanceModifiers& Mods);

    UFUNCTION(BlueprintCallable, Category = "Enemy Spawner")
    void SpawnCircleAroundPlayer(FName Type, int32 Count, const FEnemyInstanceModifiers& Mods, float Radius = 1000.f);

    UFUNCTION(BlueprintCallable, Category = "Enemy Spawner")
    AEnemyBase* SpawnOne(FName Type, const FTransform& Transform, const FEnemyInstanceModifiers& Mods);

    UFUNCTION(BlueprintCallable, Category = "Enemy Spawner")
    void SetEnemyConfig(UEnemyConfig* Config);

    UFUNCTION(BlueprintCallable, Category = "Enemy Spawner")
    UEnemyConfig* GetEnemyConfig() const { return CurrentEnemyConfig; }

    // Função para testar bosses individualmente
    UFUNCTION(BlueprintCallable, Category = "Enemy Spawner|Testing")
    void SpawnTestBoss(FName BossType);

    // Função para spawnar todos os bosses em sequência rápida para teste
    UFUNCTION(BlueprintCallable, Category = "Enemy Spawner|Testing")
    void SpawnAllBossesForTesting();

    bool IsBossEncounterActive() const { return bBossEncounterActive; }
    ABossEnemy* GetActiveBoss() const { return ActiveBoss.Get(); }
    const FBossSpawnEntry& GetActiveBossEntry() const { return ActiveBossEntry; }

    FSpawnerBossWarningSignature OnBossWarning;
    FSpawnerBossSpawnSignature OnBossSpawned;
    FSpawnerBossEndSignature OnBossEnded;
    FSpawnerBossHealthSignature OnBossHealthChanged;
    FSpawnerBossPhaseSignature OnBossPhaseChanged;
    FSpawnerBossTelegraphSignature OnBossTelegraph;
    FSpawnerBossAttackSignature OnBossAttackExecuted;

private:
    void ScheduleEvent(const FSpawnEvent& Event);
    void ExecuteSpawnEvent(const FSpawnEvent& Event);

    void ScheduleBossEvent(const FBossSpawnEntry& BossEvent);
    void TriggerBossWarning(FBossSpawnEntry BossEvent);
    void BeginBossEncounter(FBossSpawnEntry BossEvent);
    UFUNCTION()
    void HandleActiveBossDefeated(ABossEnemy* Boss);
    UFUNCTION()
    void HandleActiveBossHealthChanged(float NormalizedHealth);
    UFUNCTION()
    void HandleActiveBossPhaseChanged(int32 PhaseIndex, const FBossPhaseDefinition& PhaseData);
    UFUNCTION()
    void HandleActiveBossTelegraph(const FBossAttackPattern& Pattern);
    UFUNCTION()
    void HandleActiveBossAttack(const FBossAttackPattern& Pattern);
    void HandleMassBossSpawned(const FBossMassHandle& BossHandle, const FTransform& SpawnTransform);
    void HandleMassBossHealthChanged(const FBossMassHandle& BossHandle, float NormalizedHealth);
    void HandleMassBossDefeated(const FBossMassHandle& BossHandle);
    void ResumeDeferredEvents();
    void PauseRegularSpawns();
    void ResumeRegularSpawns(float DelaySeconds);
    void OnBossResumeTimerElapsed();
    FTransform BuildBossSpawnTransform(const FBossSpawnEntry& BossEvent) const;

    bool FindSpawnPointLinear(FVector& OutLocation);
    bool FindSpawnPointCircle(float Radius, int32 Index, int32 TotalCount, FVector& OutLocation);

    AEnemyBase* CreateEnemyActor(FName Type, const FTransform& Transform);

    void ClearBossDelegates();

    UPROPERTY()
    TObjectPtr<UEnemyConfig> CurrentEnemyConfig;

    UPROPERTY()
    TObjectPtr<const USpawnTimeline> ActiveTimeline;

    FRandomStream SpawnRng;
    TArray<FTimerHandle> ScheduledTimers;
    TArray<FTimerHandle> ScheduledBossTimers;

    TArray<FSpawnEvent> DeferredEvents;

    bool bBossEncounterActive = false;
    bool bRegularSpawnsPaused = false;

    TWeakObjectPtr<ABossEnemy> ActiveBoss;
    FBossSpawnEntry ActiveBossEntry;
    FBossMassHandle ActiveMassBossHandle;

    FTimerHandle BossResumeHandle;

    // Spawn parameters - VISIBLE RANGE FOR PROPER GAMEPLAY
    UPROPERTY(EditAnywhere, Category = "Spawn Settings")
    float LinearSpawnMinDistance = 200.f;

    UPROPERTY(EditAnywhere, Category = "Spawn Settings")
    float LinearSpawnMaxDistance = 400.f;

    UPROPERTY(EditAnywhere, Category = "Spawn Settings")
    float SpawnHeight = 100.f;

    UPROPERTY(EditAnywhere, Category = "Boss Settings")
    float BossSpawnHeightOffset = 50.f;

    UPROPERTY(EditAnywhere, Category = "Boss Settings")
    float BossSpawnForwardDistance = 1200.f;

    UPROPERTY(EditAnywhere, Category = "Boss Settings")
    float BossResumeDelayBuffer = 3.f;

    // Enemy class mappings
    UPROPERTY(EditAnywhere, Category = "Enemy Classes")
    TMap<FName, TSubclassOf<AEnemyBase>> EnemyClasses;

    void InitializeEnemyClasses();
    APawn* GetPlayerPawn() const;

    /** Cached Mass-based spawner bridge. */
    TWeakObjectPtr<UEnemyMassSpawnerSubsystem> MassSpawner;

    bool bMassBossEncounter = false;

    bool ShouldUseMassSpawning() const;
};

