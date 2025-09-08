#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "XPComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnXPChanged, float, NewXP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelChanged, int32, NewLevel);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VAZIO_API UXPComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UXPComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XP") int32 CurrentLevel = 1;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "XP") float CurrentXP = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XP") float XPToNextLevel = 100.f;

    UPROPERTY(BlueprintAssignable, Category = "XP") FOnXPChanged OnXPChanged;
    UPROPERTY(BlueprintAssignable, Category = "XP") FOnLevelChanged OnLevelChanged;

    UFUNCTION(BlueprintCallable, Category = "XP") void  AddXP(float Amount);
    UFUNCTION(BlueprintCallable, Category = "XP") float GetXPPercent() const;
};
