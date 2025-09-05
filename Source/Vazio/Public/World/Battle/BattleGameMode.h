#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BattleGameMode.generated.h"

/** GameMode para o mapa de batalha. Garante PlayerStart e usa o mesmo pawn/controller do City. */
UCLASS()
class VAZIO_API ABattleGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ABattleGameMode();

protected:
    virtual void BeginPlay() override;
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

    UFUNCTION()
    void CreatePlayerStartIfNeeded();

    // Simple environment so Battle_Main is visible even if empty
    UFUNCTION() void CreateBattleGround();
    UFUNCTION() void CreateBasicLighting();

private:
    // Cached assets resolved in constructor (legal place for FObjectFinder)
    UPROPERTY() class UStaticMesh* CachedCubeMesh = nullptr;
    UPROPERTY() class UMaterial*   CachedBasicMaterial = nullptr;
};
