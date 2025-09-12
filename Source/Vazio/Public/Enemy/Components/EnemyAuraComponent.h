#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyAuraComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VAZIO_API UEnemyAuraComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnemyAuraComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Enemy Aura")
    void SetAuraProperties(float NewRadius, float NewDPS);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aura")
    float Radius = 400.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aura")
    float DPS = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aura")
    bool bAuraActive = true;

private:
    void DealAuraDamage(float DeltaTime);
    void FindTargetsInRadius(TArray<AActor*>& OutTargets);
    
    UPROPERTY(EditAnywhere, Category = "Aura")
    float DamageTickRate = 0.5f; // Damage every 0.5 seconds
    
    float DamageAccumulator = 0.f;
};
