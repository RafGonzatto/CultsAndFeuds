#include "Enemy/Components/EnemyDropComponent.h"
#include "Enemy/EnemyBase.h"
#include "Enemy/Types/BossEnemy.h"
#include "World/Common/Collectables/XPOrb.h"
#include "Economy/GameEconomyService.h"
#include "Engine/World.h"
#include "Logging/VazioLogFacade.h"

UEnemyDropComponent::UEnemyDropComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyDropComponent::DropOnDeath(const AEnemyBase* Enemy, const FEnemyArchetype& Arch, const FEnemyInstanceModifiers& Mods, bool bIsParent)
{
    if (!Enemy)
    {
        return;
    }

    FVector DropLocation = Enemy->GetActorLocation();
    
    // Handle boss-specific rewards
    if (const ABossEnemy* Boss = Cast<ABossEnemy>(Enemy))
    {
        HandleBossRewards(Boss);
    }
    
    // Calculate and spawn XP orbs
    int32 XPAmount = CalculateXPDrop(Arch, Mods, bIsParent);
    if (XPAmount > 0)
    {
        SpawnXPOrbs(XPAmount, DropLocation);
    }
    
    // Calculate and spawn gold
    int32 GoldAmount = CalculateGoldDrop(Arch, Mods, bIsParent);
    if (GoldAmount > 0)
    {
        SpawnGold(GoldAmount, DropLocation);
    }
}

void UEnemyDropComponent::SpawnXPOrbs(int32 TotalXP, const FVector& Location)
{
    if (TotalXP <= 0 || !GetWorld())
    {
        return;
    }
    
    LOG_ENEMIES(Debug, TEXT("[XP-DROP] Spawning %d XP orbs at location %s"), TotalXP, *Location.ToString());
    
    // Determine how many orbs to spawn based on total XP
    int32 NumOrbs = 1;
    int32 XPPerOrb = TotalXP;
    
    // Split XP into multiple orbs if amount is large
    if (TotalXP > 50)
    {
        NumOrbs = FMath::Min(5, TotalXP / 10); // Max 5 orbs, min 10 XP per orb
        XPPerOrb = TotalXP / NumOrbs;
    }
    
    // Spawn the XP orbs in a small circle around the drop location
    for (int32 i = 0; i < NumOrbs; i++)
    {
        // Calculate spawn position in a circle
        float Angle = (2.0f * PI * i) / NumOrbs;
        float Radius = NumOrbs > 1 ? 50.0f : 0.0f; // Spread orbs in 50 unit radius if multiple
        FVector SpawnLocation = Location + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            10.0f // Slightly above ground
        );
        
        // Spawn the XP orb
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        LOG_ENEMIES(Debug, TEXT("[XP-DROP] Attempting to spawn XPOrb at %s"), *SpawnLocation.ToString());
        
        AXPOrb* XPOrb = GetWorld()->SpawnActor<AXPOrb>(AXPOrb::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
        if (XPOrb && IsValid(XPOrb))
        {
            XPOrb->XPAmount = XPPerOrb + (i < (TotalXP % NumOrbs) ? 1 : 0); // Distribute remainder evenly
            LOG_ENEMIES(Info, TEXT("[XP-DROP] Successfully spawned XPOrb with %d XP at %s"), XPOrb->XPAmount, *SpawnLocation.ToString());
        }
        else
        {
            LOG_ENEMIES(Error, TEXT("[XP-DROP] Failed to spawn XPOrb at %s - XPOrb is null or invalid"), *SpawnLocation.ToString());
        }
    }
}

void UEnemyDropComponent::SpawnGold(int32 GoldAmount, const FVector& Location)
{
    // TODO: Implement gold spawning logic
    LOG_ENEMIES(Debug, TEXT("Should spawn %d gold at location %s"), GoldAmount, *Location.ToString());
}

void UEnemyDropComponent::HandleBossRewards(const ABossEnemy* Boss)
{
    // TODO: Implement boss-specific reward logic
    LOG_ENEMIES(Debug, TEXT("Should handle boss rewards for boss"));
}

int32 UEnemyDropComponent::CalculateXPDrop(const FEnemyArchetype& Arch, const FEnemyInstanceModifiers& Mods, bool bIsParent)
{
    // Base XP calculation with meaningful values
    int32 BaseXP = FMath::Max(Arch.BaseRewards.XPValue, 5); // Minimum 5 XP
    
    // Scale XP based on enemy HP (stronger enemies give more XP)
    float HPMultiplier = FMath::Max(1.0f, Arch.BaseHP / 100.0f); // 1x at 100 HP, 2x at 200 HP, etc.
    BaseXP = FMath::RoundToInt(BaseXP * HPMultiplier);
    
    // Parent enemies (those that split) give bonus XP
    if (bIsParent)
    {
        BaseXP *= 2;
    }
    
    // Apply modifiers (RewardMultiplier affects XP rewards)
    BaseXP = FMath::RoundToInt(BaseXP * Mods.RewardMultiplier);
    
    // Ensure minimum viable XP drop
    BaseXP = FMath::Max(BaseXP, 5);
    
    LOG_ENEMIES(Debug, TEXT("[XP-CALC] Enemy XP: Base=%d, HP=%.0f, Parent=%s, Final=%d"),
        Arch.BaseRewards.XPValue, Arch.BaseHP, bIsParent ? TEXT("YES") : TEXT("NO"), BaseXP);
    
    return BaseXP;
}

int32 UEnemyDropComponent::CalculateGoldDrop(const FEnemyArchetype& Arch, const FEnemyInstanceModifiers& Mods, bool bIsParent)
{
    // TODO: Implement proper gold calculation
    return Arch.BaseRewards.GoldValue * (bIsParent ? 2 : 1);
}
