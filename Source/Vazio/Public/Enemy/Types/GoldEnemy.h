#pragma once

#include "CoreMinimal.h"
#include "Enemy/EnemyBase.h"
#include "GoldEnemy.generated.h"

UCLASS()
class VAZIO_API AGoldEnemy : public AEnemyBase
{
    GENERATED_BODY()

public:
    AGoldEnemy();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void PursuePlayer(float DeltaTime);
    
    UPROPERTY(EditAnywhere, Category = "AI")
    float PursuitSpeed = 210.f; // 70% of normal speed
    
    UPROPERTY(EditAnywhere, Category = "AI")
    float StoppingDistance = 100.f;
};
