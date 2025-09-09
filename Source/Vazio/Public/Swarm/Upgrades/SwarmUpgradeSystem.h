#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"
#include "SwarmUpgradeSystem.generated.h"

UENUM()
enum class ESwarmUpgradeType : uint8
{
    HealthBoost = 0,
    SpeedBoost,
    XPMultiplier
};

USTRUCT()
struct FSwarmUpgrade
{
    GENERATED_BODY()

    UPROPERTY()
    ESwarmUpgradeType Type = ESwarmUpgradeType::HealthBoost;

    UPROPERTY()
    FString Title;

    UPROPERTY()
    FString Description;

    UPROPERTY()
    FLinearColor IconColor = FLinearColor::White;

    // Generic numeric value (e.g., +HP amount, +speed %, +XP flat bonus, etc.)
    UPROPERTY()
    float Value = 0.f;

    UPROPERTY()
    int32 TimesApplied = 0;
};

class SLevelUpModal; // forward

UCLASS()
class VAZIO_API USwarmUpgradeSystem : public UObject
{
    GENERATED_BODY()
public:
    USwarmUpgradeSystem();

    void Initialize(class APlayerController* InController);
    void TriggerLevelUp();

private:
    void ProcessNextLevelUp();
    void OnUpgradeSelected(ESwarmUpgradeType SelectedType);
    void CloseLevelUpUI();
    void ApplyUpgrade(ESwarmUpgradeType UpgradeType);
    TArray<FSwarmUpgrade> GetRandomUpgrades(int32 Count);
    FSwarmUpgrade* FindUpgrade(ESwarmUpgradeType Type);

private:
    UPROPERTY() APlayerController* PlayerController = nullptr;
    TSharedPtr<SLevelUpModal> ActiveLevelUpModal;

    // Queue of pending level ups (one int per level gained)
    TArray<int32> PendingLevelUps;

    // Static set of all available upgrades (can be extended later)
    UPROPERTY() TArray<FSwarmUpgrade> AvailableUpgrades;
};