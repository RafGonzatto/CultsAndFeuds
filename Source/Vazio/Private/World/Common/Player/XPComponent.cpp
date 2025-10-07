#include "World/Common/Player/XPComponent.h"
#include "World/Common/Player/MyPlayerController.h"

UXPComponent::UXPComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UXPComponent::BeginPlay()
{
    Super::BeginPlay();
    XPToNextLevel = CalculateXPForNextLevel();
    OnXPChanged.Broadcast(CurrentXP, XPToNextLevel);
    OnLevelChanged.Broadcast(CurrentLevel);
}

void UXPComponent::AddXP(int32 Amount)
{
    if (Amount <= 0) return;

    // Apply additive multiplier (each +1 acts like +100% of base per unit if intended differently adjust formula)
    int32 FinalAmount = Amount + FMath::RoundToInt(Amount * XPMultiplier);
    CurrentXP += FinalAmount;
    UE_LOG(LogTemp, Log, TEXT("XP Added: %d (base: %d, multiplier: %.2f)"), FinalAmount, Amount, XPMultiplier);

    bool bLeveled = false;
    while (CurrentXP >= XPToNextLevel)
    {
        CurrentXP -= XPToNextLevel;
        CurrentLevel++;
        XPToNextLevel = CalculateXPForNextLevel();
        bLeveled = true;
        
        // Broadcast level up - MyCharacter will handle showing upgrade modal
        OnLevelChanged.Broadcast(CurrentLevel);
        UE_LOG(LogTemp, Warning, TEXT("[XPComponent] 🎉 Level Up! New Level: %d"), CurrentLevel);
    }

    OnXPChanged.Broadcast(CurrentXP, XPToNextLevel);
    if (bLeveled)
    {
        UE_LOG(LogTemp, Warning, TEXT("Level Up! New Level: %d"), CurrentLevel);
    }
}

void UXPComponent::AddXPMultiplier(float AdditionalMultiplier)
{
    XPMultiplier += AdditionalMultiplier;
    UE_LOG(LogTemp, Log, TEXT("XP Multiplier updated to: %.2f"), XPMultiplier);
}

int32 UXPComponent::CalculateXPForNextLevel()
{
    return FMath::RoundToInt(100 * FMath::Pow(XPMultiplierPerLevel, CurrentLevel - 1));
}