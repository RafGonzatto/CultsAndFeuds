#include "UI/DamageTextService.h"
#include "Engine/World.h"

void UDamageTextService::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UDamageTextService::ShowDamageText(const FDamageTextInfo& DamageInfo)
{
    OnShowDamageText(DamageInfo);
}

void UDamageTextService::ShowDamageNumber(float Damage, const FVector& WorldLocation, bool bIsCritical)
{
    FDamageTextInfo DamageInfo;
    DamageInfo.WorldLocation = WorldLocation;
    DamageInfo.DamageText = FString::Printf(TEXT("%.0f"), Damage);
    DamageInfo.TextColor = bIsCritical ? FLinearColor::Yellow : FLinearColor::Red;
    DamageInfo.Duration = bIsCritical ? 2.5f : 2.0f;
    DamageInfo.bIsCritical = bIsCritical;

    ShowDamageText(DamageInfo);
}

void UDamageTextService::ShowHealText(float HealAmount, const FVector& WorldLocation)
{
    FDamageTextInfo HealInfo;
    HealInfo.WorldLocation = WorldLocation;
    HealInfo.DamageText = FString::Printf(TEXT("+%.0f"), HealAmount);
    HealInfo.TextColor = FLinearColor::Green;
    HealInfo.Duration = 2.0f;
    HealInfo.bIsCritical = false;

    ShowDamageText(HealInfo);
}
