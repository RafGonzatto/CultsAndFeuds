#pragma once

#include "CoreMinimal.h"
#include "Enemy/EnemyBase.h"
#include "AuraEnemy.generated.h"

UCLASS()
class VAZIO_API AAuraEnemy : public AEnemyBase
{
    GENERATED_BODY()

public:
    AAuraEnemy();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void PursuePlayer(float DeltaTime);
    
    UPROPERTY(EditAnywhere, Category = "AI")
    float PursuitSpeed = 250.f;
    
    UPROPERTY(EditAnywhere, Category = "AI")
    float StoppingDistance = 300.f;
};
