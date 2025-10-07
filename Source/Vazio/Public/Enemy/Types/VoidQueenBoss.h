#pragma once

#include "CoreMinimal.h"
#include "Enemy/Types/BossEnemy.h"
#include "VoidQueenBoss.generated.h"

UCLASS()
class VAZIO_API AVoidQueenBoss : public ABossEnemy
{
    GENERATED_BODY()

public:
    AVoidQueenBoss();

protected:
    virtual void HandlePhaseStarted(const FBossPhaseDefinition& Phase) override;
    virtual void PerformAttackPattern(const FBossAttackPattern& Pattern) override;
    virtual void PerformMovementPattern(float DeltaTime) override;

private:
    void PerformTentacleSlam();
    void PerformVoidDash();
    void PerformBroodSummon();

    void DashTowardsPlayer(float Distance, float HeightOffset);

    float SlamRadius;
    float SlamDamage;
    float DashDistance;
    float DashSpeedMultiplier;
};

