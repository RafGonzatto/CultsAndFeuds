#pragma once

#include "CoreMinimal.h"
#include "Enemy/EnemyBase.h"
#include "RangedEnemy.generated.h"

UCLASS()
class VAZIO_API ARangedEnemy : public AEnemyBase
{
    GENERATED_BODY()

public:
    ARangedEnemy();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void HandleRangedCombat(float DeltaTime);
    void FireProjectile();
    
    UPROPERTY(EditAnywhere, Category = "Combat")
    float AttackRange = 800.f;
    
    UPROPERTY(EditAnywhere, Category = "Combat")
    float FireRate = 2.f;
    
    UPROPERTY(EditAnywhere, Category = "Combat")
    float OptimalDistance = 600.f;
    
    UPROPERTY(EditAnywhere, Category = "Combat")
    TSubclassOf<AActor> ProjectileClass;
    
    FTimerHandle FireTimerHandle;
    float LastFireTime = 0.f;
};
