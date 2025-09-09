#include "UI/HUD/SHealthBar.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SOverlay.h"
#include "Styling/SlateColor.h"
#include "Framework/Application/SlateApplication.h"

void SHealthBar::Construct(const FArguments& InArgs)
{
    CurrentHealth = 100.0f;
    MaxHealth = 100.0f;
    
    ChildSlot
    [
        SNew(SBox)
        .WidthOverride(300.0f)
        .HeightOverride(40.0f)
        [
            SNew(SBorder)
            .BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.8f))
            .BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
            .Padding(FMargin(5.0f))
            [
                SNew(SOverlay)
                
                // Progress Bar Background
                + SOverlay::Slot()
                [
                    SNew(SBorder)
                    .BorderBackgroundColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f))
                    .BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
                ]
                
                // Health Progress Bar
                + SOverlay::Slot()
                [
                    SAssignNew(HealthProgressBar, SProgressBar)
                    .Percent(this, &SHealthBar::GetHealthPercent)
                    .FillColorAndOpacity(this, &SHealthBar::GetHealthColor)
                ]
                
                // Health Text
                + SOverlay::Slot()
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                [
                    SAssignNew(HealthText, STextBlock)
                    .Text(this, &SHealthBar::GetHealthText)
                    .Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
                    .ColorAndOpacity(FLinearColor::White)
                    .ShadowOffset(FVector2D(1.0f, 1.0f))
                    .ShadowColorAndOpacity(FLinearColor::Black)
                ]
            ]
        ]
    ];
}

void SHealthBar::UpdateHealth(float NewCurrentHealth, float NewMaxHealth)
{
    CurrentHealth = NewCurrentHealth;
    MaxHealth = NewMaxHealth;
    
    // Force refresh of the widget
    if (HealthProgressBar.IsValid())
    {
        HealthProgressBar->Invalidate(EInvalidateWidgetReason::Layout);
    }
    if (HealthText.IsValid())
    {
        HealthText->Invalidate(EInvalidateWidgetReason::Layout);
    }
}

TOptional<float> SHealthBar::GetHealthPercent() const
{
    if (MaxHealth > 0.0f)
    {
        return CurrentHealth / MaxHealth;
    }
    return 0.0f;
}

FSlateColor SHealthBar::GetHealthColor() const
{
    float HealthPercent = (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
    
    if (HealthPercent > 0.6f)
    {
        // Green when healthy
        return FLinearColor(0.0f, 0.8f, 0.0f, 1.0f);
    }
    else if (HealthPercent > 0.3f)
    {
        // Yellow when damaged
        return FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
    }
    else
    {
        // Red when critical
        return FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
    }
}

FText SHealthBar::GetHealthText() const
{
    return FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurrentHealth, MaxHealth));
}
