#include "UI/Widgets/InteractPromptWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"

TSharedRef<SWidget> UInteractPromptWidget::RebuildWidget()
{
    return SNew(SBorder)
        .BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
        .Padding(6.f)
        .BorderBackgroundColor(FLinearColor(0.f,0.f,0.f,0.6f))
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("E")))
            .Justification(ETextJustify::Center)
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
        ];
}
