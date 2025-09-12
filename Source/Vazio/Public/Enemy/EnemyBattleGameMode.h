#pragma once

#include "CoreMinimal.h"
#include "World/Battle/BattleGameMode.h"
#include "Economy/GameEconomyService.h"
#include "EnemyBattleGameMode.generated.h"

class UEnemySpawnerSubsystem;
class UEnemyConfig;

UCLASS()
class VAZIO_API AEnemyBattleGameMode : public ABattleGameMode, public IGameEconomyService
{
    GENERATED_BODY()

public:
    AEnemyBattleGameMode();

protected:
    virtual void BeginPlay() override;

public:
    // IGameEconomyService interface
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Economy")
    void AddGold_Implementation(int32 Amount);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Economy")
    void SpawnXPOrbs_Implementation(int32 TotalXP, const FVector& Location);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Economy")
    int32 GetCurrentGold_Implementation() const;

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Economy")
    int32 GetCurrentXP_Implementation() const;

protected:

    UPROPERTY(BlueprintReadWrite, Category = "Economy")
    int32 PlayerGold = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Economy")
    int32 PlayerXP = 0;

private:
    void InitializeEnemySystem();
};
