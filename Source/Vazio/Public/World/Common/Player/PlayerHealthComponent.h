#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "PlayerHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDamaged, float, NewHealth);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VAZIO_API UPlayerHealthComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UPlayerHealthComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Health") float MaxHealth = 100.f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health") float CurrentHealth = 0.f;

    UPROPERTY(BlueprintAssignable, Category="Health") FOnPlayerDamaged OnDamaged;
    UPROPERTY(BlueprintAssignable, Category="Health") FOnPlayerDeath   OnDeath;

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category="Health") void  ReceiveDamage(float DamageAmount);
    UFUNCTION(BlueprintCallable, Category="Health") float GetHealthPercent() const;
};
