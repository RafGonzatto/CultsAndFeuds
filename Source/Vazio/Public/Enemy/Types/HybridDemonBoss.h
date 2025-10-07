#pragma once

#include "CoreMinimal.h"
#include "Enemy/Types/BossEnemy.h"
#include "HybridDemonBoss.generated.h"

UCLASS()
class VAZIO_API AHybridDemonBoss : public ABossEnemy
{
    GENERATED_BODY()

public:
    AHybridDemonBoss();

protected:
    virtual void HandlePhaseStarted(const FBossPhaseDefinition& Phase) override;
    virtual void PerformAttackPattern(const FBossAttackPattern& Pattern) override;
    virtual void PerformMovementPattern(float DeltaTime) override;

private:
    void PerformInfernoRain();
    void PerformShadowDash();
    void PerformOblivionProjectiles();
    void PerformCataclysm();

    float InfernoDamage;
    float InfernoRadius;
    float ShadowDashDistance;
    float CataclysmDamage;
};

