#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HUDSubsystem.generated.h"

class SHUDRoot;
class UEnemySpawnerSubsystem;
class ABossEnemy;
struct FBossSpawnEntry;
struct FBossPhaseDefinition;
struct FBossAttackPattern;

UCLASS()
class VAZIO_API UHUDSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ShowHUD();

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void HideHUD();

    UFUNCTION(BlueprintCallable, Category = "HUD")
    bool IsHUDVisible() const { return bIsHUDVisible; }

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateHealth(float CurrentHealth, float MaxHealth);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateXP(int32 CurrentXP, int32 XPToNextLevel);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateLevel(int32 NewLevel);

private:
    void SetupDelegateBindings();
    void CleanupDelegateBindings();

    void BindBossDelegates(UEnemySpawnerSubsystem* Spawner);
    void UnbindBossDelegates();

    void HandleBossSpawned(ABossEnemy* Boss, const FBossSpawnEntry& Entry);
    void HandleBossEnded();
    void HandleBossHealthChanged(float NormalizedHealth, ABossEnemy* Boss);
    void HandleBossPhaseChanged(int32 PhaseIndex, const FBossPhaseDefinition& PhaseDefinition);
    void HandleBossWarning(const FBossSpawnEntry& Entry);
    void HandleBossTelegraph(const FBossAttackPattern& Pattern);

    TSharedPtr<SHUDRoot> HUDWidget;
    bool bIsHUDVisible = false;

    TWeakObjectPtr<UEnemySpawnerSubsystem> ObservedSpawner;

    FDelegateHandle BossSpawnedHandle;
    FDelegateHandle BossEndedHandle;
    FDelegateHandle BossHealthHandle;
    FDelegateHandle BossPhaseHandle;
    FDelegateHandle BossWarningHandle;
    FDelegateHandle BossTelegraphHandle;

    int32 CachedBossPhaseCount = 0;
    FText CachedBossName;
};

