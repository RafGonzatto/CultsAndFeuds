#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerHealthComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float /*Current*/, float /*Max*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamaged, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VAZIO_API UPlayerHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerHealthComponent();

    UFUNCTION(BlueprintCallable, Category="Health")
    void ReceiveDamage(float DamageAmount); // renamed for external callers

    UFUNCTION(BlueprintCallable, Category="Health")
    float GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintCallable, Category="Health")
    float GetMaxHealth() const { return MaxHealth; }

    UFUNCTION(BlueprintCallable, Category="Health")
    bool IsAlive() const { return CurrentHealth > 0.f; }

    UFUNCTION(BlueprintCallable, Category="Health")
    float GetHealthPercent() const { return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f; }

    // Upgrade APIs
    void SetMaxHealth(float NewMaxHealth);
    void Heal(float Amount);

    // Native delegates (non-dynamic)
    FOnHealthChanged OnHealthChanged;

    // Dynamic delegates for UI / BP (even if we avoid BP, keeps API consistent)
    UPROPERTY(BlueprintAssignable, Category="Health")
    FOnDamaged OnDamaged;

    UPROPERTY(BlueprintAssignable, Category="Health")
    FOnDeath OnDeath;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, Category="Health")
    float MaxHealth = 100.f;

    UPROPERTY(VisibleAnywhere, Category="Health")
    float CurrentHealth = 0.f;
};