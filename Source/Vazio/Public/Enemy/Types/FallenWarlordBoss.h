#pragma once

#include "CoreMinimal.h"
#include "Enemy/Types/BossEnemy.h"
#include "FallenWarlordBoss.generated.h"

UCLASS()
class VAZIO_API AFallenWarlordBoss : public ABossEnemy
{
    GENERATED_BODY()

public:
    AFallenWarlordBoss();

protected:
    virtual void BeginPlay() override;
    virtual void HandlePhaseStarted(const FBossPhaseDefinition& Phase) override;
    virtual void PerformAttackPattern(const FBossAttackPattern& Pattern) override;
    virtual void PerformMovementPattern(float DeltaTime) override;

protected:
    // TODO: Weapon system - AFallenSwordWeapon class needs to be created first
    // UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    // TSubclassOf<AFallenSwordWeapon> SwordWeaponClass;

    // UPROPERTY()
    // TObjectPtr<AFallenSwordWeapon> CurrentWeapon;

private:
    void PerformGreatswordSweep();
    void PerformFlameWave();
    void PerformEarthShatter(); 

    float SweepDamage;
    float FlameWaveDamage;
    float FlameWaveRadius;
    float EarthShatterRange;
};

