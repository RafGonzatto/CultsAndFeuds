#include "UI/HUD/SLevelText.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/SlateColor.h"
#include "Framework/Application/SlateApplication.h"

void SLevelText::Construct(const FArguments& InArgs)
{
    CurrentLevel = 1;
    
    ChildSlot
    [
        SNew(SBox)
        .WidthOverride(120.0f)
        .HeightOverride(50.0f)
        [
            SNew(SBorder)
            .BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.8f))
            .BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
            .Padding(FMargin(10.0f))
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            [
                SAssignNew(LevelTextBlock, STextBlock)
                .Text(this, &SLevelText::GetLevelText)
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
                .ColorAndOpacity(FLinearColor(1.0f, 0.8f, 0.0f, 1.0f)) // Golden color
                .ShadowOffset(FVector2D(2.0f, 2.0f))
                .ShadowColorAndOpacity(FLinearColor::Black)
                .Justification(ETextJustify::Center)
            ]
        ]
    ];
}

void SLevelText::UpdateLevel(int32 NewLevel)
{
    CurrentLevel = NewLevel;
    
    // Force refresh of the widget
    if (LevelTextBlock.IsValid())
    {
        LevelTextBlock->Invalidate(EInvalidateWidgetReason::Layout);
    }
}

FText SLevelText::GetLevelText() const
{
    return FText::FromString(FString::Printf(TEXT("LVL %d"), CurrentLevel));
}
