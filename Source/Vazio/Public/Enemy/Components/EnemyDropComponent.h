#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enemy/EnemyTypes.h"
#include "EnemyDropComponent.generated.h"

class AEnemyBase;
class IGameEconomyService;
class ABossEnemy;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VAZIO_API UEnemyDropComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnemyDropComponent();

    UFUNCTION(BlueprintCallable, Category = "Enemy Drop")
    void DropOnDeath(const AEnemyBase* Enemy, const FEnemyArchetype& Arch, const FEnemyInstanceModifiers& Mods, bool bIsParent);

private:
    void SpawnXPOrbs(int32 TotalXP, const FVector& Location);
    void SpawnGold(int32 GoldAmount, const FVector& Location);
    void HandleBossRewards(const ABossEnemy* Boss);

    int32 CalculateXPDrop(const FEnemyArchetype& Arch, const FEnemyInstanceModifiers& Mods, bool bIsParent);
    int32 CalculateGoldDrop(const FEnemyArchetype& Arch, const FEnemyInstanceModifiers& Mods, bool bIsParent);

    UPROPERTY(EditAnywhere, Category = "Drop Settings")
    int32 MaxXPOrbs = 5;

    UPROPERTY(EditAnywhere, Category = "Drop Settings")
    float OrbSpreadRadius = 100.f;

    UPROPERTY(EditAnywhere, Category = "Drop Settings")
    float DifficultyScaleXP = 1.f;

    UPROPERTY(EditAnywhere, Category = "Boss Rewards")
    int32 BossXPReward = 600;

    UPROPERTY(EditAnywhere, Category = "Boss Rewards")
    int32 BossGoldReward = 40;

    UPROPERTY(EditAnywhere, Category = "Boss Rewards")
    FText UniqueUpgradeMessage;
};

