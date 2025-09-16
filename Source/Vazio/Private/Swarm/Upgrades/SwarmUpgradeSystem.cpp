#include "Swarm/Upgrades/SwarmUpgradeSystem.h"
#include "UI/LevelUp/SLevelUpModal.h"
#include "World/Common/Player/MyCharacter.h"
#include "World/Common/Player/PlayerHealthComponent.h"
#include "World/Common/Player/XPComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Swarm/Weapons/SwarmWeaponBase.h"
#include "Swarm/Weapons/SwarmPistolWeapon.h"
#include "Swarm/Weapons/SwarmSpinningSawsWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"

USwarmUpgradeSystem::USwarmUpgradeSystem()
{
    // Initialize upgrade definitions
    FSwarmUpgrade HealthUpgrade; HealthUpgrade.Type = ESwarmUpgradeType::HealthBoost; HealthUpgrade.Title = TEXT("+10 HP"); HealthUpgrade.Description = TEXT("Aumenta vida maxima em 10 e cura 10 HP"); HealthUpgrade.IconColor = FLinearColor::Red; HealthUpgrade.Value = 10.0f; AvailableUpgrades.Add(HealthUpgrade);
    FSwarmUpgrade SpeedUpgrade; SpeedUpgrade.Type = ESwarmUpgradeType::SpeedBoost; SpeedUpgrade.Title = TEXT("+5% Velocidade"); SpeedUpgrade.Description = TEXT("Aumenta velocidade de movimento em 5%"); SpeedUpgrade.IconColor = FLinearColor::Green; SpeedUpgrade.Value = 0.05f; AvailableUpgrades.Add(SpeedUpgrade);
    FSwarmUpgrade XPUpgrade; XPUpgrade.Type = ESwarmUpgradeType::XPMultiplier; XPUpgrade.Title = TEXT("XP +1 por orbe"); XPUpgrade.Description = TEXT("Cada orbe de XP vale +1 adicional"); XPUpgrade.IconColor = FLinearColor::Blue; XPUpgrade.Value = 1.0f; AvailableUpgrades.Add(XPUpgrade);
    FSwarmUpgrade PistolUpgrade; PistolUpgrade.Type = ESwarmUpgradeType::WeaponPistol; PistolUpgrade.Title = TEXT("Desbloquear Pistola"); PistolUpgrade.Description = TEXT("Desbloqueia a pistola. Depois concede +5 de dano"); PistolUpgrade.IconColor = FLinearColor::Yellow; PistolUpgrade.Value = 5.0f; AvailableUpgrades.Add(PistolUpgrade);
    FSwarmUpgrade SawUpgrade; SawUpgrade.Type = ESwarmUpgradeType::WeaponSpinningSaws; SawUpgrade.Title = TEXT("Desbloquear Serras"); SawUpgrade.Description = TEXT("Desbloqueia serras giratorias. Depois concede +4 de dano"); SawUpgrade.IconColor = FLinearColor(0.75f, 0.75f, 0.75f, 1.0f); SawUpgrade.Value = 4.0f; AvailableUpgrades.Add(SawUpgrade);
}

void USwarmUpgradeSystem::Initialize(APlayerController* InController)
{
    PlayerController = InController;
    UE_LOG(LogTemp, Warning, TEXT("SwarmUpgradeSystem: Initialized"));
}

void USwarmUpgradeSystem::TriggerLevelUp()
{
    if (!PlayerController || !PlayerController->GetWorld())
    {
        return;
    }

    // Check if we're in Battle_Main level
    FString LevelName = PlayerController->GetWorld()->GetMapName();
    if (!LevelName.Contains(TEXT("Battle_Main")))
    {
        UE_LOG(LogTemp, Warning, TEXT("SwarmUpgradeSystem: Not in Battle_Main, ignoring level up"));
        return;
    }

    PendingLevelUps.Add(1);
    UE_LOG(LogTemp, Warning, TEXT("LevelUp:Queued Total=%d"), PendingLevelUps.Num());

    if (PendingLevelUps.Num() == 1)
    {
        ProcessNextLevelUp();
    }
}

void USwarmUpgradeSystem::ProcessNextLevelUp()
{
    if (PendingLevelUps.Num() == 0)
    {
        return;
    }

    // Pause game
    UGameplayStatics::SetGamePaused(PlayerController->GetWorld(), true);
    UE_LOG(LogTemp, Warning, TEXT("LevelUp:Paused"));

    // Create and show Slate level up modal
    if (!ActiveLevelUpModal.IsValid())
    {
        TArray<FSwarmUpgrade> RandomUpgrades = GetRandomUpgrades(3);
        
        ActiveLevelUpModal = SNew(SLevelUpModal)
            .OnUpgradeChosen(FOnUpgradeChosen::CreateUObject(this, &USwarmUpgradeSystem::OnUpgradeSelected));

        ActiveLevelUpModal->SetupUpgrades(RandomUpgrades);

        // Add to viewport
        if (UGameViewportClient* ViewportClient = PlayerController->GetWorld()->GetGameViewport())
        {
            ViewportClient->AddViewportWidgetContent(ActiveLevelUpModal.ToSharedRef(), 1000);
        }

        // Set input mode and focus
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(ActiveLevelUpModal);
        PlayerController->SetInputMode(InputMode);
        PlayerController->bShowMouseCursor = true;

        // Set keyboard focus
        FSlateApplication::Get().SetKeyboardFocus(ActiveLevelUpModal);

        UE_LOG(LogTemp, Warning, TEXT("LevelUp:UI:SlateModalOpened"));
        
        FString OptionsStr;
        for (const FSwarmUpgrade& Upgrade : RandomUpgrades)
        {
            OptionsStr += FString::Printf(TEXT("%s,"), *Upgrade.Title);
        }
        UE_LOG(LogTemp, Warning, TEXT("LevelUp:OptionsShown=[%s]"), *OptionsStr);
    }
}

void USwarmUpgradeSystem::OnUpgradeSelected(ESwarmUpgradeType SelectedType)
{
    UE_LOG(LogTemp, Warning, TEXT("LevelUp:Selected=%d"), (int32)SelectedType);
    ApplyUpgrade(SelectedType);
    CloseLevelUpUI();
}

void USwarmUpgradeSystem::CloseLevelUpUI()
{
    if (ActiveLevelUpModal.IsValid())
    {
        // Remove from viewport
        if (UGameViewportClient* ViewportClient = PlayerController->GetWorld()->GetGameViewport())
        {
            ViewportClient->RemoveViewportWidgetContent(ActiveLevelUpModal.ToSharedRef());
        }
        ActiveLevelUpModal.Reset();
    }

    // Resume game
    UGameplayStatics::SetGamePaused(PlayerController->GetWorld(), false);
    
    // Reset input mode
    FInputModeGameOnly InputMode;
    PlayerController->SetInputMode(InputMode);
    PlayerController->bShowMouseCursor = false;

    UE_LOG(LogTemp, Warning, TEXT("LevelUp:UI:Closed QueueRemaining=%d"), PendingLevelUps.Num() - 1);

    // Remove processed level up
    if (PendingLevelUps.Num() > 0)
    {
        PendingLevelUps.RemoveAt(0);
    }

    // Process next level up if any
    if (PendingLevelUps.Num() > 0)
    {
        FTimerHandle TimerHandle;
        PlayerController->GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
            [this]() { ProcessNextLevelUp(); },
            0.1f,
            false
        );
    }
}

void USwarmUpgradeSystem::ApplyUpgrade(ESwarmUpgradeType UpgradeType)
{
    AMyCharacter* Character = Cast<AMyCharacter>(PlayerController->GetPawn());
    if (!Character)
    {
        return;
    }

    FSwarmUpgrade* Upgrade = FindUpgrade(UpgradeType);
    if (!Upgrade)
    {
        return;
    }

    switch (UpgradeType)
    {
    case ESwarmUpgradeType::HealthBoost:
        {
            UPlayerHealthComponent* HealthComp = Character->FindComponentByClass<UPlayerHealthComponent>();
            if (HealthComp)
            {
                float OldMax = HealthComp->GetMaxHealth();
                HealthComp->SetMaxHealth(OldMax + Upgrade->Value);
                HealthComp->Heal(Upgrade->Value);
                UE_LOG(LogTemp, Warning, TEXT("LevelUp:Applied=HealthBoost +%f"), Upgrade->Value);
            }
        }
        break;

    case ESwarmUpgradeType::SpeedBoost:
        {
            UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
            if (MoveComp)
            {
                float CurrentSpeed = MoveComp->MaxWalkSpeed;
                MoveComp->MaxWalkSpeed = CurrentSpeed * (1.0f + Upgrade->Value);
                UE_LOG(LogTemp, Warning, TEXT("LevelUp:Applied=SpeedBoost +%f%%"), Upgrade->Value * 100);
            }
        }
        break;

    case ESwarmUpgradeType::XPMultiplier:
        {
            UXPComponent* XPComp = Character->FindComponentByClass<UXPComponent>();
            if (XPComp)
            {
                XPComp->AddXPMultiplier(Upgrade->Value);
                UE_LOG(LogTemp, Warning, TEXT("LevelUp:Applied=XPMultiplier +%f"), Upgrade->Value);
            }
        }
        break;

    case ESwarmUpgradeType::WeaponPistol:
        {
            const TSubclassOf<ASwarmWeaponBase> WeaponClass = ASwarmPistolWeapon::StaticClass();
            if (!Character->HasWeaponOfClass(WeaponClass))
            {
                if (Character->AddWeapon(WeaponClass))
                {
                    UE_LOG(LogTemp, Warning, TEXT("LevelUp:Applied=Pistol Unlock"));
                }
            }
            else if (ASwarmWeaponBase* Weapon = Character->FindWeaponByClass(WeaponClass))
            {
                Weapon->ApplyUpgrade(Upgrade->Value);
                UE_LOG(LogTemp, Warning, TEXT("LevelUp:Applied=Pistol +%f Dano"), Upgrade->Value);
            }
        }
        break;

    case ESwarmUpgradeType::WeaponSpinningSaws:
        {
            const TSubclassOf<ASwarmWeaponBase> WeaponClass = ASwarmSpinningSawsWeapon::StaticClass();
            if (!Character->HasWeaponOfClass(WeaponClass))
            {
                if (Character->AddWeapon(WeaponClass))
                {
                    UE_LOG(LogTemp, Warning, TEXT("LevelUp:Applied=Saws Unlock"));
                }
            }
            else if (ASwarmWeaponBase* Weapon = Character->FindWeaponByClass(WeaponClass))
            {
                Weapon->ApplyUpgrade(Upgrade->Value);
                UE_LOG(LogTemp, Warning, TEXT("LevelUp:Applied=Saws +%f Dano"), Upgrade->Value);
            }
        }
        break;
    }

    Upgrade->TimesApplied++;
}

TArray<FSwarmUpgrade> USwarmUpgradeSystem::GetRandomUpgrades(int32 Count)
{
    TArray<FSwarmUpgrade> Result;
    TArray<FSwarmUpgrade> TempUpgrades = AvailableUpgrades;

    Count = FMath::Min(Count, TempUpgrades.Num());
    
    for (int32 i = 0; i < Count; i++)
    {
        int32 RandomIndex = FMath::RandRange(0, TempUpgrades.Num() - 1);
        FSwarmUpgrade UpgradeCopy = TempUpgrades[RandomIndex];
        RefreshUpgradeDisplayData(UpgradeCopy);
        Result.Add(UpgradeCopy);
        TempUpgrades.RemoveAt(RandomIndex);
    }

    return Result;
}

void USwarmUpgradeSystem::RefreshUpgradeDisplayData(FSwarmUpgrade& Upgrade) const
{
    const AMyCharacter* Character = PlayerController ? Cast<AMyCharacter>(PlayerController->GetPawn()) : nullptr;

    auto ConfigureWeaponUpgrade = [&](TSubclassOf<ASwarmWeaponBase> WeaponClass, const TCHAR* UnlockTitle, const TCHAR* UnlockDescription, const TCHAR* BaseName, const TCHAR* UpgradeDescription)
    {
        const bool bHasWeapon = Character && Character->HasWeaponOfClass(WeaponClass);
        const bool bAlreadyApplied = Upgrade.TimesApplied > 0 || bHasWeapon;

        if (!bAlreadyApplied)
        {
            Upgrade.Title = UnlockTitle;
            Upgrade.Description = UnlockDescription;
        }
        else
        {
            // Build title manually to avoid non-literal format string issues
            Upgrade.Title = FString::Printf(TEXT("%s +%.0f Dano"), BaseName, Upgrade.Value);
            Upgrade.Description = UpgradeDescription;
        }
    };

    switch (Upgrade.Type)
    {
    case ESwarmUpgradeType::WeaponPistol:
        ConfigureWeaponUpgrade(ASwarmPistolWeapon::StaticClass(), TEXT("Desbloquear Pistola"), TEXT("Desbloqueia a pistola, disparando projeteis em inimigos."), TEXT("Pistola"), TEXT("Aumenta o dano base da pistola."));
        break;
    case ESwarmUpgradeType::WeaponSpinningSaws:
        ConfigureWeaponUpgrade(ASwarmSpinningSawsWeapon::StaticClass(), TEXT("Desbloquear Serras"), TEXT("Desbloqueia serras giratorias que orbitam o campeao."), TEXT("Serras"), TEXT("Aumenta o dano periodico das serras."));
        break;
    default:
        break;
    }
}

FSwarmUpgrade* USwarmUpgradeSystem::FindUpgrade(ESwarmUpgradeType Type)
{
    for (FSwarmUpgrade& Upgrade : AvailableUpgrades)
    {
        if (Upgrade.Type == Type)
        {
            return &Upgrade;
        }
    }
    return nullptr;
}
