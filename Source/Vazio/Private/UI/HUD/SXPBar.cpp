#include "UI/HUD/SXPBar.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SOverlay.h"
#include "Styling/SlateColor.h"
#include "Framework/Application/SlateApplication.h"

void SXPBar::Construct(const FArguments& InArgs)
{
    CurrentXP = 0;
    XPToNextLevel = 100;
    
    ChildSlot
    [
        SNew(SBox)
        .WidthOverride(400.0f)
        .HeightOverride(25.0f)
        [
            SNew(SBorder)
            .BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.8f))
            .BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
            .Padding(FMargin(3.0f))
            [
                SNew(SOverlay)
                
                // Progress Bar Background
                + SOverlay::Slot()
                [
                    SNew(SBorder)
                    .BorderBackgroundColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f))
                    .BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
                ]
                
                // XP Progress Bar
                + SOverlay::Slot()
                [
                    SAssignNew(XPProgressBar, SProgressBar)
                    .Percent(this, &SXPBar::GetXPPercent)
                    .FillColorAndOpacity(FLinearColor(0.0f, 0.6f, 1.0f, 1.0f)) // Blue XP bar
                ]
                
                // XP Text
                + SOverlay::Slot()
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                [
                    SAssignNew(XPText, STextBlock)
                    .Text(this, &SXPBar::GetXPText)
                    .Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
                    .ColorAndOpacity(FLinearColor::White)
                    .ShadowOffset(FVector2D(1.0f, 1.0f))
                    .ShadowColorAndOpacity(FLinearColor::Black)
                ]
            ]
        ]
    ];
}

void SXPBar::UpdateXP(int32 NewCurrentXP, int32 NewXPToNextLevel)
{
    CurrentXP = NewCurrentXP;
    XPToNextLevel = NewXPToNextLevel;
    
    // Force refresh of the widget
    if (XPProgressBar.IsValid())
    {
        XPProgressBar->Invalidate(EInvalidateWidgetReason::Layout);
    }
    if (XPText.IsValid())
    {
        XPText->Invalidate(EInvalidateWidgetReason::Layout);
    }
}

TOptional<float> SXPBar::GetXPPercent() const
{
    if (XPToNextLevel > 0)
    {
        return static_cast<float>(CurrentXP) / static_cast<float>(XPToNextLevel);
    }
    return 0.0f;
}

FText SXPBar::GetXPText() const
{
    return FText::FromString(FString::Printf(TEXT("XP: %d / %d"), CurrentXP, XPToNextLevel));
}
