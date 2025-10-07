#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BattleGameMode.generated.h"

class UEnemyConfig;

/** GameMode para o mapa de batalha. Garante PlayerStart e usa o mesmo pawn/controller do City. */
UCLASS()
class VAZIO_API ABattleGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ABattleGameMode();

    // Enemy system functions (public so they can be called from console/PlayerController)
    UFUNCTION(BlueprintCallable, Category = "Enemy Spawn")
    void StartEnemyWave(const FString& JSONWaveData, int32 Seed = 0);

    UFUNCTION(BlueprintCallable, Category = "Enemy Spawn")
    void StartDefaultWave();

    UFUNCTION(BlueprintCallable, Category = "Enemy Spawn")
    void StartTestWave();

protected:
    virtual void BeginPlay() override;
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

    UFUNCTION()
    void CreatePlayerStartIfNeeded();

    // Simple environment so Battle_Main is visible even if empty
    UFUNCTION() void CreateBattleGround();
    UFUNCTION() void CreateBasicLighting();

    // Enemy system integration
    UFUNCTION() void InitializeEnemySystem();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Config")
    TObjectPtr<class UEnemyConfig> DefaultEnemyConfig;

private:
    // Cached assets resolved in constructor (legal place for FObjectFinder)
    UPROPERTY() class UStaticMesh* CachedCubeMesh = nullptr;
    UPROPERTY() class UMaterial*   CachedBasicMaterial = nullptr;
};
