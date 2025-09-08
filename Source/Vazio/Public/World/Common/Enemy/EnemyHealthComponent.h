#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyHealthComponent.generated.h"

class AXPOrb;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VAZIO_API UEnemyHealthComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UEnemyHealthComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Health") float MaxHealth = 50.f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health") float CurrentHealth = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="XP") int32 XPValue = 10;

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category="Health") void ReceiveDamage(float DamageAmount);
};
