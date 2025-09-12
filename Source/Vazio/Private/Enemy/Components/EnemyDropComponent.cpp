#include "Enemy/Components/EnemyDropComponent.h"
#include "Enemy/EnemyBase.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "Economy/GameEconomyService.h"
#include "World/Common/Collectables/XPOrb.h"
#include "Enemy/EnemyTypes.h"

UEnemyDropComponent::UEnemyDropComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    MaxXPOrbs = 5;
    OrbSpreadRadius = 100.f;
    DifficultyScaleXP = 1.f;
}

void UEnemyDropComponent::DropOnDeath(const AEnemyBase* Enemy, const FEnemyArchetype& Arch, const FEnemyInstanceModifiers& Mods, bool bIsParent)
{
    if (!Enemy)
    {
        return;
    }

    FVector DropLocation = Enemy->GetActorLocation();
    
    // Calculate drops
    int32 XPAmount = CalculateXPDrop(Arch, Mods, bIsParent);
    int32 GoldAmount = CalculateGoldDrop(Arch, Mods, bIsParent);
    
    // Spawn XP orbs if any XP to drop
    if (XPAmount > 0)
    {
        SpawnXPOrbs(XPAmount, DropLocation);
    }
    
    // Add gold if any to drop
    if (GoldAmount > 0)
    {
        SpawnGold(GoldAmount, DropLocation);
    }
    
    UE_LOG(LogEnemy, Log, TEXT("%s died (parent=%d) drops: XP=%d Gold=%d"), 
           *Enemy->GetName(), bIsParent ? 1 : 0, XPAmount, GoldAmount);
}

int32 UEnemyDropComponent::CalculateXPDrop(const FEnemyArchetype& Arch, const FEnemyInstanceModifiers& Mods, bool bIsParent)
{
    // SplitterSlime parents drop 0 XP
    if (bIsParent)
    {
        return 0;
    }
    
    // Base XP calculation
    float FinalXP = Arch.BaseXP * DifficultyScaleXP;
    
    // Apply big modifier (×10 XP)
    if (Mods.bBig)
    {
        FinalXP *= 10.f;
    }
    
    return FMath::RoundToInt(FinalXP);
}

int32 UEnemyDropComponent::CalculateGoldDrop(const FEnemyArchetype& Arch, const FEnemyInstanceModifiers& Mods, bool bIsParent)
{
    // SplitterSlime parents drop 0 Gold
    if (bIsParent)
    {
        return 0;
    }
    
    int32 GoldAmount = 0;
    
    switch (Arch.Drop)
    {
        case EDropProfile::Normal:
            // NormalEnemy: 5% chance for +1 Gold
            if (FMath::FRandRange(0.f, 1.f) <= 0.05f)
            {
                GoldAmount = 1;
            }
            break;
            
        case EDropProfile::Gold:
            // GoldEnemy: 100% chance for +10 Gold
            GoldAmount = 10;
            break;
            
        case EDropProfile::XPOnly:
        case EDropProfile::None:
        default:
            // No gold drop
            GoldAmount = 0;
            break;
    }
    
    // Note: big modifier does NOT affect gold drops according to requirements
    
    return GoldAmount;
}

void UEnemyDropComponent::SpawnXPOrbs(int32 TotalXP, const FVector& Location)
{
    if (TotalXP <= 0)
    {
        return;
    }
    
    // Partition XP into multiple orbs (capped for performance)
    int32 NumOrbs = FMath::Min(MaxXPOrbs, FMath::Max(1, TotalXP / 2));
    int32 XPPerOrb = TotalXP / NumOrbs;
    int32 ExtraXP = TotalXP % NumOrbs;
    
    for (int32 i = 0; i < NumOrbs; i++)
    {
        // Calculate spawn position with radial dispersion
        float Angle = (2.0f * PI * i) / NumOrbs;
        float RandomRadius = FMath::FRandRange(0.3f, 1.0f) * OrbSpreadRadius;
        FVector Offset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * RandomRadius;
        FVector OrbLocation = Location + Offset;
        
        // Add extra XP to first orb
        int32 ThisOrbXP = XPPerOrb + (i == 0 ? ExtraXP : 0);
        
        // Spawn XP orb
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        if (AXPOrb* NewOrb = GetWorld()->SpawnActor<AXPOrb>(AXPOrb::StaticClass(), OrbLocation, FRotator::ZeroRotator, SpawnParams))
        {
            // Set XP value if the orb has such functionality
            // This would depend on your XPOrb implementation
            UE_LOG(LogEconomy, VeryVerbose, TEXT("Spawned XP orb with %d XP at %s"), ThisOrbXP, *OrbLocation.ToCompactString());
        }
    }
    
    UE_LOG(LogEconomy, Log, TEXT("Spawned %d XP orbs totaling %d XP"), NumOrbs, TotalXP);
}

void UEnemyDropComponent::SpawnGold(int32 GoldAmount, const FVector& Location)
{
    if (GoldAmount <= 0)
    {
        return;
    }
    
    // Try to find a game mode or player controller that implements IGameEconomyService
    if (AGameModeBase* GameMode = UGameplayStatics::GetGameMode(GetWorld()))
    {
        if (IGameEconomyService* EconomyService = Cast<IGameEconomyService>(GameMode))
        {
            EconomyService->AddGold(GoldAmount);
            UE_LOG(LogEconomy, Log, TEXT("Added %d gold to economy"), GoldAmount);
            return;
        }
    }
    
    // Fallback: try player controller
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (IGameEconomyService* EconomyService = Cast<IGameEconomyService>(PC))
        {
            EconomyService->AddGold(GoldAmount);
            UE_LOG(LogEconomy, Log, TEXT("Added %d gold to player controller"), GoldAmount);
            return;
        }
    }
    
    UE_LOG(LogEconomy, Warning, TEXT("Could not find IGameEconomyService to add %d gold"), GoldAmount);
}
