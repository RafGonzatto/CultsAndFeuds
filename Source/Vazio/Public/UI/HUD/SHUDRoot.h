#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SHealthBar;
class SXPBar;
class SLevelText;

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

    // SWidget interface
    virtual bool SupportsKeyboardFocus() const override { return false; }

private:
    TSharedPtr<SHealthBar> HealthBar;
    TSharedPtr<SXPBar> XPBar;
    TSharedPtr<SLevelText> LevelText;
};
