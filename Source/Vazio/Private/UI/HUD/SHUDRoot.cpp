#include "UI/HUD/SHUDRoot.h"
#include "UI/HUD/SHealthBar.h"
#include "UI/HUD/SXPBar.h"
#include "UI/HUD/SLevelText.h"
#include "UI/HUD/SBossHealthBar.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SOverlay.h"
#include "Framework/Application/SlateApplication.h"

void SHUDRoot::Construct(const FArguments& InArgs)
{
    ChildSlot
    [
        SNew(SOverlay)

        // Top-left corner: Health Bar
        + SOverlay::Slot()
        .HAlign(HAlign_Left)
        .VAlign(VAlign_Top)
        .Padding(20.0f)
        [
            SAssignNew(HealthBar, SHealthBar)
        ]

        // Top-center: Boss health bar
        + SOverlay::Slot()
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Top)
        .Padding(0.0f, 10.0f, 0.0f, 0.0f)
        [
            SAssignNew(BossHealthBar, SBossHealthBar)
        ]

        // Slightly below boss bar: Level Display
        + SOverlay::Slot()
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Top)
        .Padding(0.0f, 80.0f, 0.0f, 0.0f)
        [
            SAssignNew(LevelText, SLevelText)
        ]

        // Bottom-center: XP Bar
        + SOverlay::Slot()
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Bottom)
        .Padding(0.0f, 0.0f, 0.0f, 50.0f)
        [
            SAssignNew(XPBar, SXPBar)
        ]
    ];

    if (BossHealthBar.IsValid())
    {
        BossHealthBar->SetBossVisible(false);
    }
}

void SHUDRoot::UpdateHealth(float CurrentHealth, float MaxHealth)
{
    if (HealthBar.IsValid())
    {
        HealthBar->UpdateHealth(CurrentHealth, MaxHealth);
    }
}

void SHUDRoot::UpdateXP(int32 CurrentXP, int32 XPToNextLevel)
{
    if (XPBar.IsValid())
    {
        XPBar->UpdateXP(CurrentXP, XPToNextLevel);
    }
}

void SHUDRoot::UpdateLevel(int32 NewLevel)
{
    if (LevelText.IsValid())
    {
        LevelText->UpdateLevel(NewLevel);
    }
}

void SHUDRoot::ShowBoss(const FText& BossName, float HealthPercent, int32 PhaseIndex, int32 PhaseCount)
{
    if (BossHealthBar.IsValid())
    {
        BossHealthBar->SetBossVisible(true);
        BossHealthBar->UpdateBossInfo(BossName, HealthPercent, PhaseIndex, PhaseCount);
    }
}

void SHUDRoot::UpdateBossHealth(float HealthPercent)
{
    if (BossHealthBar.IsValid())
    {
        BossHealthBar->UpdateHealth(HealthPercent);
    }
}

void SHUDRoot::UpdateBossPhase(int32 PhaseIndex, int32 PhaseCount)
{
    if (BossHealthBar.IsValid())
    {
        BossHealthBar->UpdatePhase(PhaseIndex, PhaseCount);
    }
}

void SHUDRoot::HideBoss()
{
    if (BossHealthBar.IsValid())
    {
        BossHealthBar->SetBossVisible(false);
    }
}

void SHUDRoot::ShowBossWarning(const FText& WarningText)
{
    if (BossHealthBar.IsValid())
    {
        BossHealthBar->SetWarning(WarningText);
    }
}