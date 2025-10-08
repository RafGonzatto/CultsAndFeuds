#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DamageTextService.generated.h"

USTRUCT(BlueprintType)
struct VAZIOSHARED_API FDamageTextInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DamageText = TEXT("0");

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor TextColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 2.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCritical = false;

    FDamageTextInfo()
    {
        WorldLocation = FVector::ZeroVector;
        DamageText = TEXT("0");
        TextColor = FLinearColor::Red;
        Duration = 2.f;
        bIsCritical = false;
    }
};

UCLASS()
class VAZIOSHARED_API UDamageTextService : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Damage Text")
    void ShowDamageText(const FDamageTextInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Damage Text")
    void ShowDamageNumber(float Damage, const FVector& WorldLocation, bool bIsCritical = false);

    UFUNCTION(BlueprintCallable, Category = "Damage Text")
    void ShowHealText(float HealAmount, const FVector& WorldLocation);

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Damage Text")
    void OnShowDamageText(const FDamageTextInfo& DamageInfo);
};
