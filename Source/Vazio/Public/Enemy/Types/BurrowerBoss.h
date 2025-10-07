#pragma once

#include "CoreMinimal.h"
#include "Enemy/Types/BossEnemy.h"
#include "BurrowerBoss.generated.h"

UCLASS()
class VAZIO_API ABurrowerBoss : public ABossEnemy
{
    GENERATED_BODY()

public:
    ABurrowerBoss();

protected:
    virtual void HandlePhaseStarted(const FBossPhaseDefinition& Phase) override;
    virtual void PerformAttackPattern(const FBossAttackPattern& Pattern) override;
    virtual void PerformMovementPattern(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

private:
    void StartBurrow();
    void FinishBurrow();
    void PerformAmbushStrike();
    void PerformSpikeBurst();

    bool bIsBurrowed;
    float BurrowDuration;
    float SpikeDamage;
    float SpikeRadius;

    FTimerHandle BurrowTimerHandle;
};

