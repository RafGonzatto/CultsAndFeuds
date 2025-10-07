#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SHealthBar;
class SXPBar;
class SLevelText;
class SBossHealthBar;

class VAZIO_API SHUDRoot : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SHUDRoot) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    // Update methods
    void UpdateHealth(float CurrentHealth, float MaxHealth);
    void UpdateXP(int32 CurrentXP, int32 XPToNextLevel);
    void UpdateLevel(int32 NewLevel);

    void ShowBoss(const FText& BossName, float HealthPercent, int32 PhaseIndex, int32 PhaseCount);
    void UpdateBossHealth(float HealthPercent);
    void UpdateBossPhase(int32 PhaseIndex, int32 PhaseCount);
    void HideBoss();
    void ShowBossWarning(const FText& WarningText);

    // SWidget interface
    virtual bool SupportsKeyboardFocus() const override { return false; }

private:
    TSharedPtr<SHealthBar> HealthBar;
    TSharedPtr<SXPBar> XPBar;
    TSharedPtr<SLevelText> LevelText;
    TSharedPtr<SBossHealthBar> BossHealthBar;
};
