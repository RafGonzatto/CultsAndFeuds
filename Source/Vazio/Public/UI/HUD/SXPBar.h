#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class VAZIO_API SXPBar : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SXPBar) {}
        SLATE_ARGUMENT(FText, Label)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void UpdateXP(int32 CurrentXP, int32 XPToNextLevel);

private:
    TSharedPtr<class SProgressBar> XPProgressBar;
    TSharedPtr<class STextBlock> XPText;
    
    // XP data
    int32 CurrentXP = 0;
    int32 XPToNextLevel = 100;
    
    // Binding methods
    TOptional<float> GetXPPercent() const;
    FText GetXPText() const;

    void UpdateVisuals(int32 Current, int32 Needed);
};
