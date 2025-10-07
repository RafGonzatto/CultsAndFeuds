#include "Gameplay/Upgrades/UpgradeSystem.h"
#include "World/Common/Player/MyCharacter.h"
#include "World/Common/Player/PlayerHealthComponent.h"
#include "World/Common/Player/XPComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

void UUpgradeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeUpgrades();
    UE_LOG(LogTemp, Log, TEXT("[UpgradeSubsystem] Initialized"));
}

void UUpgradeSubsystem::Deinitialize()
{
    UpgradeLevels.Empty();
    AvailableUpgrades.Empty();
    Super::Deinitialize();
}

void UUpgradeSubsystem::InitializeUpgrades()
{
    AvailableUpgrades.Empty();

    // Weapon Upgrades
    {
        FUpgradeData WeaponDamage;
        WeaponDamage.Type = EUpgradeType::WeaponDamage;
        WeaponDamage.DisplayName = FText::FromString("Weapon Damage");
        WeaponDamage.Description = FText::FromString("Increase damage dealt by all weapons");
        WeaponDamage.Value = 5.0f; // +5 damage per level
        WeaponDamage.IconColor = FLinearColor::Red;
        AvailableUpgrades.Add(WeaponDamage);
    }

    {
        FUpgradeData FireRate;
        FireRate.Type = EUpgradeType::WeaponFireRate;
        FireRate.DisplayName = FText::FromString("Fire Rate");
        FireRate.Description = FText::FromString("Increase weapon attack speed");
        FireRate.Value = 10.0f; // +10% fire rate per level
        FireRate.IconColor = FLinearColor::Yellow;
        AvailableUpgrades.Add(FireRate);
    }

    {
        FUpgradeData Range;
        Range.Type = EUpgradeType::WeaponRange;
        Range.DisplayName = FText::FromString("Attack Range");
        Range.Description = FText::FromString("Increase weapon range");
        Range.Value = 50.0f; // +50 units per level
        Range.IconColor = FLinearColor(1.0f, 0.5f, 0.0f); // Orange
        AvailableUpgrades.Add(Range);
    }

    // Player Stats
    {
        FUpgradeData Speed;
        Speed.Type = EUpgradeType::MovementSpeed;
        Speed.DisplayName = FText::FromString("Movement Speed");
        Speed.Description = FText::FromString("Increase movement speed");
        Speed.Value = 30.0f; // +30 units per level
        Speed.IconColor = FLinearColor::Green;
        AvailableUpgrades.Add(Speed);
    }

    {
        FUpgradeData Health;
        Health.Type = EUpgradeType::MaxHealth;
        Health.DisplayName = FText::FromString("Max Health");
        Health.Description = FText::FromString("Increase maximum health");
        Health.Value = 20.0f; // +20 HP per level
        Health.IconColor = FLinearColor(0.0f, 1.0f, 0.5f); // Cyan-Green
        AvailableUpgrades.Add(Health);
    }

    {
        FUpgradeData Regen;
        Regen.Type = EUpgradeType::HealthRegen;
        Regen.DisplayName = FText::FromString("Health Regeneration");
        Regen.Description = FText::FromString("Regenerate health over time");
        Regen.Value = 2.0f; // +2 HP per second per level
        Regen.IconColor = FLinearColor(0.5f, 1.0f, 0.5f); // Light Green
        AvailableUpgrades.Add(Regen);
    }

    // Combat
    {
        FUpgradeData Crit;
        Crit.Type = EUpgradeType::CriticalChance;
        Crit.DisplayName = FText::FromString("Critical Chance");
        Crit.Description = FText::FromString("Increase chance for critical hits");
        Crit.Value = 5.0f; // +5% crit chance per level
        Crit.IconColor = FLinearColor(1.0f, 0.0f, 0.5f); // Pink
        AvailableUpgrades.Add(Crit);
    }

    {
        FUpgradeData CritDmg;
        CritDmg.Type = EUpgradeType::CriticalDamage;
        CritDmg.DisplayName = FText::FromString("Critical Damage");
        CritDmg.Description = FText::FromString("Increase critical hit damage multiplier");
        CritDmg.Value = 25.0f; // +25% crit damage per level
        CritDmg.IconColor = FLinearColor(1.0f, 0.0f, 0.0f); // Bright Red
        AvailableUpgrades.Add(CritDmg);
    }

    // Utility
    {
        FUpgradeData XP;
        XP.Type = EUpgradeType::XPGain;
        XP.DisplayName = FText::FromString("XP Multiplier");
        XP.Description = FText::FromString("Gain more experience from kills");
        XP.Value = 10.0f; // +10% XP per level
        XP.IconColor = FLinearColor(0.5f, 0.5f, 1.0f); // Light Blue
        AvailableUpgrades.Add(XP);
    }

    {
        FUpgradeData Pickup;
        Pickup.Type = EUpgradeType::PickupRange;
        Pickup.DisplayName = FText::FromString("Pickup Range");
        Pickup.Description = FText::FromString("Collect items from further away");
        Pickup.Value = 50.0f; // +50 units per level
        Pickup.IconColor = FLinearColor(1.0f, 1.0f, 0.0f); // Yellow
        AvailableUpgrades.Add(Pickup);
    }

    UE_LOG(LogTemp, Log, TEXT("[UpgradeSubsystem] Initialized %d upgrade types"), AvailableUpgrades.Num());
}

TArray<FUpgradeData> UUpgradeSubsystem::GenerateRandomUpgrades(int32 Count)
{
    TArray<FUpgradeData> Result;
    TArray<FUpgradeData> EligibleUpgrades;

    // Filter out maxed upgrades
    for (const FUpgradeData& Upgrade : AvailableUpgrades)
    {
        if (!IsUpgradeMaxed(Upgrade.Type))
        {
            EligibleUpgrades.Add(Upgrade);
        }
    }

    if (EligibleUpgrades.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UpgradeSubsystem] No eligible upgrades available!"));
        return Result;
    }

    // Randomly select upgrades
    Count = FMath::Min(Count, EligibleUpgrades.Num());
    
    for (int32 i = 0; i < Count; ++i)
    {
        int32 RandomIndex = FMath::RandRange(0, EligibleUpgrades.Num() - 1);
        FUpgradeData SelectedUpgrade = EligibleUpgrades[RandomIndex];
        
        // Update current level for display
        SelectedUpgrade.CurrentLevel = GetUpgradeLevel(SelectedUpgrade.Type);
        
        Result.Add(SelectedUpgrade);
        EligibleUpgrades.RemoveAt(RandomIndex); // Avoid duplicates
    }

    UE_LOG(LogTemp, Display, TEXT("[UpgradeSubsystem] Generated %d random upgrades"), Result.Num());
    return Result;
}

void UUpgradeSubsystem::ApplyUpgrade(EUpgradeType Type, AMyCharacter* Player)
{
    if (!Player)
    {
        UE_LOG(LogTemp, Error, TEXT("[UpgradeSubsystem] Cannot apply upgrade: Player is null"));
        return;
    }

    // Increment level
    int32& Level = UpgradeLevels.FindOrAdd(Type, 0);
    Level++;

    UE_LOG(LogTemp, Display, TEXT("[UpgradeSubsystem] Applying upgrade %d (Level %d)"), (int32)Type, Level);

    // Apply based on category
    switch (Type)
    {
        case EUpgradeType::WeaponDamage:
        case EUpgradeType::WeaponFireRate:
        case EUpgradeType::WeaponRange:
        case EUpgradeType::WeaponPiercing:
            ApplyWeaponUpgrade(Type, Player);
            break;

        case EUpgradeType::MovementSpeed:
        case EUpgradeType::MaxHealth:
        case EUpgradeType::HealthRegen:
            ApplyStatUpgrade(Type, Player);
            break;

        case EUpgradeType::CriticalChance:
        case EUpgradeType::CriticalDamage:
        case EUpgradeType::AreaDamage:
        case EUpgradeType::XPGain:
        case EUpgradeType::PickupRange:
        case EUpgradeType::DodgeChance:
            ApplyUtilityUpgrade(Type, Player);
            break;

        default:
            UE_LOG(LogTemp, Warning, TEXT("[UpgradeSubsystem] Unknown upgrade type: %d"), (int32)Type);
            break;
    }
}

void UUpgradeSubsystem::ApplyWeaponUpgrade(EUpgradeType Type, AMyCharacter* Player)
{
    float Value = CalculateUpgradeValue(Type, GetUpgradeLevel(Type));

    switch (Type)
    {
        case EUpgradeType::WeaponDamage:
            // Apply to all equipped weapons
            // TODO: Implement weapon system upgrade application
            UE_LOG(LogTemp, Display, TEXT("[UpgradeSubsystem] Weapon Damage increased by %.1f"), Value);
            break;

        case EUpgradeType::WeaponFireRate:
            UE_LOG(LogTemp, Display, TEXT("[UpgradeSubsystem] Fire Rate increased by %.1f%%"), Value);
            break;

        case EUpgradeType::WeaponRange:
            UE_LOG(LogTemp, Display, TEXT("[UpgradeSubsystem] Attack Range increased by %.1f"), Value);
            break;

        default:
            break;
    }
}

void UUpgradeSubsystem::ApplyStatUpgrade(EUpgradeType Type, AMyCharacter* Player)
{
    float Value = CalculateUpgradeValue(Type, GetUpgradeLevel(Type));

    switch (Type)
    {
        case EUpgradeType::MovementSpeed:
            if (UCharacterMovementComponent* Movement = Player->GetCharacterMovement())
            {
                Movement->MaxWalkSpeed += Value;
                UE_LOG(LogTemp, Display, TEXT("[UpgradeSubsystem] Movement Speed increased to %.1f"), Movement->MaxWalkSpeed);
            }
            break;

        case EUpgradeType::MaxHealth:
            if (UPlayerHealthComponent* HealthComp = Player->FindComponentByClass<UPlayerHealthComponent>())
            {
                HealthComp->IncreaseMaxHealth(Value);
                UE_LOG(LogTemp, Display, TEXT("[UpgradeSubsystem] Max Health increased by %.1f"), Value);
            }
            break;

        case EUpgradeType::HealthRegen:
            // TODO: Implement health regeneration system
            UE_LOG(LogTemp, Display, TEXT("[UpgradeSubsystem] Health Regen increased by %.1f HP/s"), Value);
            break;

        default:
            break;
    }
}

void UUpgradeSubsystem::ApplyUtilityUpgrade(EUpgradeType Type, AMyCharacter* Player)
{
    float Value = CalculateUpgradeValue(Type, GetUpgradeLevel(Type));

    switch (Type)
    {
        case EUpgradeType::XPGain:
            if (UXPComponent* XPComp = Player->FindComponentByClass<UXPComponent>())
            {
                // Convert percentage to multiplier (10% = 0.1)
                float MultiplierToAdd = Value / 100.0f;
                XPComp->AddXPMultiplier(MultiplierToAdd);
                UE_LOG(LogTemp, Display, TEXT("[UpgradeSubsystem] XP Gain increased by %.1f%% (multiplier: %.2f)"), 
                    Value, XPComp->GetCurrentXPMultiplier());
            }
            break;

        case EUpgradeType::PickupRange:
            // TODO: Increase pickup sphere radius
            UE_LOG(LogTemp, Display, TEXT("[UpgradeSubsystem] Pickup Range increased by %.1f"), Value);
            break;

        case EUpgradeType::CriticalChance:
            UE_LOG(LogTemp, Display, TEXT("[UpgradeSubsystem] Critical Chance increased by %.1f%%"), Value);
            break;

        case EUpgradeType::CriticalDamage:
            UE_LOG(LogTemp, Display, TEXT("[UpgradeSubsystem] Critical Damage increased by %.1f%%"), Value);
            break;

        default:
            break;
    }
}

float UUpgradeSubsystem::CalculateUpgradeValue(EUpgradeType Type, int32 Level) const
{
    // Find base value from available upgrades
    for (const FUpgradeData& Upgrade : AvailableUpgrades)
    {
        if (Upgrade.Type == Type)
        {
            return Upgrade.Value * Level;
        }
    }

    return 0.0f;
}

int32 UUpgradeSubsystem::GetUpgradeLevel(EUpgradeType Type) const
{
    const int32* Level = UpgradeLevels.Find(Type);
    return Level ? *Level : 0;
}

bool UUpgradeSubsystem::IsUpgradeMaxed(EUpgradeType Type) const
{
    int32 CurrentLevel = GetUpgradeLevel(Type);
    
    for (const FUpgradeData& Upgrade : AvailableUpgrades)
    {
        if (Upgrade.Type == Type)
        {
            return CurrentLevel >= Upgrade.MaxLevel;
        }
    }

    return false;
}

void UUpgradeSubsystem::ResetAllUpgrades()
{
    UpgradeLevels.Empty();
    UE_LOG(LogTemp, Display, TEXT("[UpgradeSubsystem] All upgrades reset"));
}

FUpgradeData UUpgradeSubsystem::GetUpgradeDisplayData(EUpgradeType Type) const
{
    for (const FUpgradeData& Upgrade : AvailableUpgrades)
    {
        if (Upgrade.Type == Type)
        {
            FUpgradeData Data = Upgrade;
            Data.CurrentLevel = GetUpgradeLevel(Type);
            return Data;
        }
    }

    return FUpgradeData();
}
