#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UpgradeSystem.generated.h"

class AMyCharacter;

/**
 * Generic upgrade types for player progression
 * Replaces ESwarmUpgradeType
 */
UENUM(BlueprintType)
enum class EUpgradeType : uint8
{
    // Weapon Upgrades
    WeaponDamage UMETA(DisplayName = "Weapon Damage"),
    WeaponFireRate UMETA(DisplayName = "Fire Rate"),
    WeaponRange UMETA(DisplayName = "Attack Range"),
    WeaponPiercing UMETA(DisplayName = "Piercing Shots"),
    
    // Player Stats
    MovementSpeed UMETA(DisplayName = "Movement Speed"),
    MaxHealth UMETA(DisplayName = "Max Health"),
    HealthRegen UMETA(DisplayName = "Health Regeneration"),
    
    // Combat
    CriticalChance UMETA(DisplayName = "Critical Chance"),
    CriticalDamage UMETA(DisplayName = "Critical Damage"),
    AreaDamage UMETA(DisplayName = "Area Damage"),
    
    // Utility
    XPGain UMETA(DisplayName = "XP Multiplier"),
    PickupRange UMETA(DisplayName = "Pickup Range"),
    DodgeChance UMETA(DisplayName = "Dodge Chance")
};

/**
 * Upgrade data structure
 * Replaces FSwarmUpgrade
 */
USTRUCT(BlueprintType)
struct FUpgradeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EUpgradeType Type = EUpgradeType::WeaponDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Value = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxLevel = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* Icon = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor IconColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TimesApplied = 0;

    FUpgradeData()
        : Type(EUpgradeType::WeaponDamage)
        , DisplayName(FText::FromString("Unknown"))
        , Description(FText::FromString("No description"))
        , Value(0.0f)
        , CurrentLevel(0)
        , MaxLevel(5)
        , Icon(nullptr)
        , IconColor(FLinearColor::White)
        , TimesApplied(0)
    {}
};

/**
 * World Subsystem for managing player upgrades
 * Replaces USwarmUpgradeSystem
 */
UCLASS()
class VAZIO_API UUpgradeSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Generate random upgrades for level up selection
     * @param Count Number of upgrades to generate (default 3)
     * @return Array of random upgrade options
     */
    UFUNCTION(BlueprintCallable, Category = "Upgrades")
    TArray<FUpgradeData> GenerateRandomUpgrades(int32 Count = 3);

    /**
     * Apply chosen upgrade to player character
     * @param Type The upgrade type to apply
     * @param Player The player character to upgrade
     */
    UFUNCTION(BlueprintCallable, Category = "Upgrades")
    void ApplyUpgrade(EUpgradeType Type, AMyCharacter* Player);

    /**
     * Get current level of a specific upgrade
     * @param Type The upgrade type to check
     * @return Current level (0 if not yet applied)
     */
    UFUNCTION(BlueprintCallable, Category = "Upgrades")
    int32 GetUpgradeLevel(EUpgradeType Type) const;

    /**
     * Check if an upgrade has reached max level
     * @param Type The upgrade type to check
     * @return True if at max level
     */
    UFUNCTION(BlueprintCallable, Category = "Upgrades")
    bool IsUpgradeMaxed(EUpgradeType Type) const;

    /**
     * Reset all upgrades (for testing or new game)
     */
    UFUNCTION(BlueprintCallable, Category = "Upgrades")
    void ResetAllUpgrades();

    /**
     * Get display data for an upgrade type
     */
    FUpgradeData GetUpgradeDisplayData(EUpgradeType Type) const;

private:
    /** Initialize all available upgrades with default values */
    void InitializeUpgrades();

    /** Apply weapon-related upgrades */
    void ApplyWeaponUpgrade(EUpgradeType Type, AMyCharacter* Player);

    /** Apply stat-related upgrades */
    void ApplyStatUpgrade(EUpgradeType Type, AMyCharacter* Player);

    /** Apply utility upgrades */
    void ApplyUtilityUpgrade(EUpgradeType Type, AMyCharacter* Player);

    /** Get upgrade value based on current level (scaling) */
    float CalculateUpgradeValue(EUpgradeType Type, int32 Level) const;

private:
    /** Track current level of each upgrade */
    UPROPERTY()
    TMap<EUpgradeType, int32> UpgradeLevels;

    /** Available upgrade templates */
    UPROPERTY()
    TArray<FUpgradeData> AvailableUpgrades;

    /** Cache of player controller for UI management */
    UPROPERTY()
    TWeakObjectPtr<APlayerController> CachedPlayerController;
};
