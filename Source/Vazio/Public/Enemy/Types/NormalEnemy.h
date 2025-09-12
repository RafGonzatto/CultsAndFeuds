#pragma once

#include "CoreMinimal.h"
#include "Enemy/EnemyBase.h"
#include "NormalEnemy.generated.h"

UCLASS()
class VAZIO_API ANormalEnemy : public AEnemyBase
{
    GENERATED_BODY()

public:
    ANormalEnemy();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void PursuePlayer(float DeltaTime);
    
    UPROPERTY(EditAnywhere, Category = "AI")
    float PursuitSpeed = 300.f;
    
    UPROPERTY(EditAnywhere, Category = "AI")
    float StoppingDistance = 100.f;
};
