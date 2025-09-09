#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class VAZIO_API SHealthBar : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SHealthBar) {}
        SLATE_ARGUMENT(FText, Label)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void UpdateHealth(float CurrentHealth, float MaxHealth);

private:
    TSharedPtr<class SProgressBar> HealthProgressBar;
    TSharedPtr<class STextBlock> HealthText;
    
    // Health data
    float CurrentHealth = 100.0f;
    float MaxHealth = 100.0f;
    
    // Binding methods
    TOptional<float> GetHealthPercent() const;
    FSlateColor GetHealthColor() const;
    FText GetHealthText() const;

    void UpdateVisuals(float Current, float Max);
};
