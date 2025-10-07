#include "UI/HUD/SBossHealthBar.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Notifications/SProgressBar.h"

void SBossHealthBar::Construct(const FArguments& InArgs)
{
    HealthPercent = 0.f;
    CachedBossName = FText::GetEmpty();
    CachedPhaseLabel = FText::GetEmpty();
    CachedWarningText = FText::GetEmpty();
    bVisible = false;

    ChildSlot
    [
        SNew(SBorder)
        .Padding(FMargin(16.f, 12.f))
        .BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.55f))
        .VAlign(VAlign_Center)
        .HAlign(HAlign_Fill)
        .Visibility(this, &SBossHealthBar::GetWidgetVisibility)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SAssignNew(NameLabel, STextBlock)
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
                .ColorAndOpacity(FLinearColor::White)
                .Text(CachedBossName)
                .Justification(ETextJustify::Center)
            ]

            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0.f, 6.f, 0.f, 0.f)
            [
                SAssignNew(HealthProgress, SProgressBar)
                .Percent(HealthPercent)
            ]

            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0.f, 4.f, 0.f, 0.f)
            [
                SAssignNew(PhaseLabel, STextBlock)
                .Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
                .ColorAndOpacity(FLinearColor::Gray)
                .Text(CachedPhaseLabel)
                .Justification(ETextJustify::Center)
            ]

            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0.f, 2.f, 0.f, 0.f)
            [
                SAssignNew(WarningLabel, STextBlock)
                .Font(FCoreStyle::GetDefaultFontStyle("Italic", 12))
                .ColorAndOpacity(FLinearColor(1.f, 0.4f, 0.2f, 0.9f))
                .Text(CachedWarningText)
                .Justification(ETextJustify::Center)
                .WrapTextAt(500.f)
            ]
        ]
    ];
}

void SBossHealthBar::UpdateBossInfo(const FText& BossName, float HealthFraction, int32 PhaseIndex, int32 PhaseCount)
{
    CachedBossName = BossName;
    UpdateHealth(HealthFraction);
    UpdatePhase(PhaseIndex, PhaseCount);

    if (NameLabel.IsValid())
    {
        NameLabel->SetText(CachedBossName);
    }
}

void SBossHealthBar::UpdateHealth(float HealthFraction)
{
    HealthPercent = FMath::Clamp(HealthFraction, 0.f, 1.f);
    if (HealthProgress.IsValid())
    {
        HealthProgress->SetPercent(HealthPercent);
    }
}

void SBossHealthBar::UpdatePhase(int32 PhaseIndex, int32 PhaseCount)
{
    CachedPhaseLabel = BuildPhaseLabel(PhaseIndex, PhaseCount);
    if (PhaseLabel.IsValid())
    {
        PhaseLabel->SetText(CachedPhaseLabel);
    }
}

void SBossHealthBar::SetWarning(const FText& WarningText)
{
    CachedWarningText = WarningText;
    if (WarningLabel.IsValid())
    {
        WarningLabel->SetText(CachedWarningText);
    }
}

void SBossHealthBar::SetBossVisible(bool bVisibleIn)
{
    bVisible = bVisibleIn;
}

EVisibility SBossHealthBar::GetWidgetVisibility() const
{
    return bVisible ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SBossHealthBar::BuildPhaseLabel(int32 PhaseIndex, int32 PhaseCount) const
{
    if (PhaseCount <= 0)
    {
        return FText::GetEmpty();
    }

    const int32 DisplayIndex = FMath::Clamp(PhaseIndex + 1, 1, PhaseCount);
    return FText::Format(NSLOCTEXT("BossHealth", "PhaseLabel", "Phase {0} / {1}"), DisplayIndex, PhaseCount);
}
