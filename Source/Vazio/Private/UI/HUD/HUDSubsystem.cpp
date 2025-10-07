#include "UI/HUD/HUDSubsystem.h"
#include "UI/HUD/SHUDRoot.h"
#include "Enemy/EnemySpawnerSubsystem.h"
#include "Enemy/Types/BossEnemy.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SViewport.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"

void UHUDSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Create the HUD widget
    HUDWidget = SNew(SHUDRoot);
    
    SetupDelegateBindings();
}

void UHUDSubsystem::Deinitialize()
{
    CleanupDelegateBindings();
    
    if (HUDWidget.IsValid())
    {
        HUDWidget.Reset();
    }
    
    Super::Deinitialize();
}

void UHUDSubsystem::ShowHUD()
{
    if (!HUDWidget.IsValid())
    {
        return;
    }

    if (!bIsHUDVisible)
    {
        if (UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport())
        {
            ViewportClient->AddViewportWidgetContent(HUDWidget.ToSharedRef(), 100);
            bIsHUDVisible = true;
        }
    }
}

void UHUDSubsystem::HideHUD()
{
    if (!HUDWidget.IsValid())
    {
        return;
    }

    if (bIsHUDVisible)
    {
        if (UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport())
        {
            ViewportClient->RemoveViewportWidgetContent(HUDWidget.ToSharedRef());
            bIsHUDVisible = false;
        }
    }
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
    // TODO: Setup delegate bindings for enemy spawner events
}

void UHUDSubsystem::CleanupDelegateBindings()
{
    // TODO: Cleanup delegate bindings
}

void UHUDSubsystem::BindBossDelegates(UEnemySpawnerSubsystem* Spawner)
{
    // TODO: Bind boss-related delegates
}

void UHUDSubsystem::UnbindBossDelegates()
{
    // TODO: Unbind boss-related delegates
}

void UHUDSubsystem::HandleBossSpawned(ABossEnemy* Boss, const FBossSpawnEntry& Entry)
{
    // TODO: Handle boss spawned event
}

void UHUDSubsystem::HandleBossEnded()
{
    // TODO: Handle boss ended event
}

void UHUDSubsystem::HandleBossHealthChanged(float NormalizedHealth, ABossEnemy* Boss)
{
    // TODO: Handle boss health changed event
}

void UHUDSubsystem::HandleBossPhaseChanged(int32 PhaseIndex, const FBossPhaseDefinition& PhaseDefinition)
{
    // TODO: Handle boss phase changed event
}

void UHUDSubsystem::HandleBossWarning(const FBossSpawnEntry& Entry)
{
    // TODO: Handle boss warning event
}

void UHUDSubsystem::HandleBossTelegraph(const FBossAttackPattern& Pattern)
{
    // TODO: Handle boss telegraph event
}
