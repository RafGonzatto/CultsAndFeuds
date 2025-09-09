#include "UI/HUD/HUDSubsystem.h"
#include "UI/HUD/SHUDRoot.h"
#include "World/Common/Player/PlayerHealthComponent.h"
#include "World/Common/Player/XPComponent.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Kismet/GameplayStatics.h"

void UHUDSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("HUDSubsystem: Initialized"));
}

void UHUDSubsystem::Deinitialize()
{
    HideHUD();
    CleanupDelegateBindings();
    
    UE_LOG(LogTemp, Log, TEXT("HUDSubsystem: Deinitialized"));
    
    Super::Deinitialize();
}

void UHUDSubsystem::ShowHUD()
{
    if (bIsHUDVisible || !FSlateApplication::IsInitialized())
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World || !World->GetGameViewport())
    {
        UE_LOG(LogTemp, Warning, TEXT("HUDSubsystem: Cannot show HUD - no valid world or viewport"));
        return;
    }

    // Create the HUD widget
    HUDWidget = SNew(SHUDRoot);
    
    if (HUDWidget.IsValid())
    {
        // Add to viewport
        World->GetGameViewport()->AddViewportWidgetContent(HUDWidget.ToSharedRef(), 100); // High Z-Order
        bIsHUDVisible = true;
        
        // Setup delegate bindings to auto-update
        SetupDelegateBindings();
        
        UE_LOG(LogTemp, Log, TEXT("HUDSubsystem: HUD shown successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("HUDSubsystem: Failed to create HUD widget"));
    }
}

void UHUDSubsystem::HideHUD()
{
    if (!bIsHUDVisible || !HUDWidget.IsValid())
    {
        return;
    }

    UWorld* World = GetWorld();
    if (World && World->GetGameViewport())
    {
        World->GetGameViewport()->RemoveViewportWidgetContent(HUDWidget.ToSharedRef());
    }
    
    CleanupDelegateBindings();
    HUDWidget.Reset();
    bIsHUDVisible = false;
    
    UE_LOG(LogTemp, Log, TEXT("HUDSubsystem: HUD hidden"));
}

void UHUDSubsystem::UpdateHealth(float CurrentHealth, float MaxHealth)
{
    if (HUDWidget.IsValid())
    {
        HUDWidget->UpdateHealth(CurrentHealth, MaxHealth);
    }
}

void UHUDSubsystem::UpdateXP(int32 CurrentXP, int32 XPToNextLevel)
{
    if (HUDWidget.IsValid())
    {
        HUDWidget->UpdateXP(CurrentXP, XPToNextLevel);
    }
}

void UHUDSubsystem::UpdateLevel(int32 NewLevel)
{
    if (HUDWidget.IsValid())
    {
        HUDWidget->UpdateLevel(NewLevel);
    }
}

void UHUDSubsystem::SetupDelegateBindings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find the player pawn and bind to its components
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("HUDSubsystem: No player pawn found for delegate binding"));
        return;
    }

    // Bind to Health Component
    if (UPlayerHealthComponent* HealthComp = PlayerPawn->FindComponentByClass<UPlayerHealthComponent>())
    {
        HealthComp->OnHealthChanged.AddUObject(this, &UHUDSubsystem::UpdateHealth);
        UE_LOG(LogTemp, Log, TEXT("HUDSubsystem: Bound to HealthComponent"));
        
        // Initial update
        UpdateHealth(HealthComp->GetCurrentHealth(), HealthComp->GetMaxHealth());
    }

    // Bind to XP Component
    if (UXPComponent* XPComp = PlayerPawn->FindComponentByClass<UXPComponent>())
    {
        XPComp->OnXPChanged.AddDynamic(this, &UHUDSubsystem::UpdateXP);
        XPComp->OnLevelChanged.AddDynamic(this, &UHUDSubsystem::UpdateLevel);
        UE_LOG(LogTemp, Log, TEXT("HUDSubsystem: Bound to XPComponent"));
        
        // Initial updates
        UpdateXP(XPComp->GetCurrentXP(), XPComp->GetXPToNextLevel());
        UpdateLevel(XPComp->GetCurrentLevel());
    }
}

void UHUDSubsystem::CleanupDelegateBindings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        return;
    }

    // Cleanup Health Component bindings
    if (UPlayerHealthComponent* HealthComp = PlayerPawn->FindComponentByClass<UPlayerHealthComponent>())
    {
        HealthComp->OnHealthChanged.RemoveAll(this);
    }

    // Cleanup XP Component bindings
    if (UXPComponent* XPComp = PlayerPawn->FindComponentByClass<UXPComponent>())
    {
        XPComp->OnXPChanged.RemoveAll(this);
        XPComp->OnLevelChanged.RemoveAll(this);
    }
    
    UE_LOG(LogTemp, Log, TEXT("HUDSubsystem: Delegate bindings cleaned up"));
}
