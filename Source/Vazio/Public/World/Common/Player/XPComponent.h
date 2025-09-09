#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "XPComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnXPChangedDyn, int32, CurrentXP, int32, XPToNextLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelChangedDyn, int32, NewLevel);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VAZIO_API UXPComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UXPComponent();

    UFUNCTION(BlueprintCallable, Category="XP")
    void AddXP(int32 Amount);

    UFUNCTION(BlueprintCallable, Category="XP")
    int32 GetCurrentXP() const { return CurrentXP; }

    UFUNCTION(BlueprintCallable, Category="XP")
    int32 GetCurrentLevel() const { return CurrentLevel; }

    UFUNCTION(BlueprintCallable, Category="XP")
    int32 GetXPToNextLevel() const { return XPToNextLevel; }

    UFUNCTION(BlueprintCallable, Category="XP")
    float GetXPPercent() const { return XPToNextLevel > 0 ? (float)CurrentXP / (float)XPToNextLevel : 0.f; }

    // Upgrade API
    void AddXPMultiplier(float AdditionalMultiplier);
    float GetCurrentXPMultiplier() const { return XPMultiplier; }

    // Dynamic Delegates for UI
    UPROPERTY(BlueprintAssignable, Category="XP")
    FOnXPChangedDyn OnXPChanged;

    UPROPERTY(BlueprintAssignable, Category="XP")
    FOnLevelChangedDyn OnLevelChanged;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, Category="XP")
    int32 CurrentXP = 0;

    UPROPERTY(EditAnywhere, Category="XP")
    int32 CurrentLevel = 1;

    UPROPERTY(EditAnywhere, Category="XP")
    int32 XPToNextLevel = 100;

    UPROPERTY(EditAnywhere, Category="XP")
    float XPMultiplierPerLevel = 1.5f;

    UPROPERTY()
    float XPMultiplier = 0.f; // additive; final gain = Amount + Amount*XPMultiplier

    int32 CalculateXPForNextLevel();
};