#pragma once

#include "CoreMinimal.h"
#include "Enemy/EnemyBase.h"
#include "DashEnemy.generated.h"

UCLASS()
class VAZIO_API ADashEnemy : public AEnemyBase
{
    GENERATED_BODY()

public:
    ADashEnemy();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void HandleDashBehavior(float DeltaTime);
    void ExecuteDash();
    
    UPROPERTY(EditAnywhere, Category = "Dash")
    float DashSpeed = 1000.f;
    
    UPROPERTY(EditAnywhere, Category = "Dash")
    float DashDuration = 0.5f;
    
    UPROPERTY(EditAnywhere, Category = "Dash")
    float MinDashDistance = 200.f;
    
    bool bIsDashing = false;
    bool bCanDash = true;
    FVector DashDirection;
    float DashTimeRemaining = 0.f;
    
    FTimerHandle DashCooldownTimerHandle;
    
    UFUNCTION()
    void OnDashCooldownComplete();
};
