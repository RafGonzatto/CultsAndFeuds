#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SProgressBar;
class STextBlock;

class VAZIO_API SBossHealthBar : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SBossHealthBar) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    void UpdateBossInfo(const FText& BossName, float HealthPercent, int32 PhaseIndex, int32 PhaseCount);
    void UpdateHealth(float HealthPercent);
    void UpdatePhase(int32 PhaseIndex, int32 PhaseCount);
    void SetWarning(const FText& WarningText);
    void SetBossVisible(bool bVisibleIn);

private:
    EVisibility GetWidgetVisibility() const;
    FText BuildPhaseLabel(int32 PhaseIndex, int32 PhaseCount) const;

    float HealthPercent;
    FText CachedBossName;
    FText CachedPhaseLabel;
    FText CachedWarningText;
    bool bVisible;

    TSharedPtr<SProgressBar> HealthProgress;
    TSharedPtr<STextBlock> NameLabel;
    TSharedPtr<STextBlock> PhaseLabel;
    TSharedPtr<STextBlock> WarningLabel;
};
