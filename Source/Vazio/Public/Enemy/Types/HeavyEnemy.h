#pragma once

#include "CoreMinimal.h"
#include "Enemy/EnemyBase.h"
#include "HeavyEnemy.generated.h"

UCLASS()
class VAZIO_API AHeavyEnemy : public AEnemyBase
{
    GENERATED_BODY()

public:
    AHeavyEnemy();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void PursuePlayer(float DeltaTime);
    
    UPROPERTY(EditAnywhere, Category = "AI")
    float HeavyPursuitSpeed = 150.f;
    
    UPROPERTY(EditAnywhere, Category = "AI")
    float StoppingDistance = 120.f;
};
