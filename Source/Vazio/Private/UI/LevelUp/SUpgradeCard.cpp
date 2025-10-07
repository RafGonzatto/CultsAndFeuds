#include "UI/LevelUp/SUpgradeCard.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SUpgradeCard::Construct(const FArguments& InArgs)
{
    CardUpgrade = InArgs._Upgrade;
    CardIndex = InArgs._CardIndex;

    ChildSlot
    [
        SAssignNew(CardBorder, SBorder)
        .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
        .BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.9f))
        .Padding(20)
        [
            SNew(SBox)
            .MinDesiredWidth(250)
            .MinDesiredHeight(350)
            [
                SNew(SVerticalBox)
                
                // Icon
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Center)
                .Padding(0, 0, 0, 20)
                [
                    SAssignNew(IconBorder, SBorder)
                    .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                    .BorderBackgroundColor(CardUpgrade.IconColor)
                    .Padding(0)
                    [
                        SNew(SBox)
                        .WidthOverride(80)
                        .HeightOverride(80)
                    ]
                ]
                
                // Title
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Center)
                .Padding(0, 0, 0, 10)
                [
                    SAssignNew(TitleText, STextBlock)
                    .Text(CardUpgrade.DisplayName)
                    .Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
                    .ColorAndOpacity(FLinearColor::Yellow)
                    .Justification(ETextJustify::Center)
                ]
                
                // Description
                + SVerticalBox::Slot()
                .FillHeight(1.0f)
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                [
                    SAssignNew(DescriptionText, STextBlock)
                    .Text(CardUpgrade.Description)
                    .Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
                    .ColorAndOpacity(FLinearColor::White)
                    .Justification(ETextJustify::Center)
                    .AutoWrapText(true)
                ]
            ]
        ]
    ];

    UpdateVisualState();
}

void SUpgradeCard::SetSelected(bool bSelected)
{
    bIsSelected = bSelected;
    UpdateVisualState();
}

void SUpgradeCard::UpdateVisualState()
{
    if (CardBorder.IsValid())
    {
        FLinearColor BorderColor;
        FVector2D Scale = FVector2D(1.0f, 1.0f);

        if (bIsSelected)
        {
            BorderColor = FLinearColor(0.3f, 0.3f, 0.1f, 1.0f);
            Scale = FVector2D(1.1f, 1.1f);
        }
        else if (bIsHovered)
        {
            BorderColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);
            Scale = FVector2D(1.05f, 1.05f);
        }
        else
        {
            BorderColor = FLinearColor(0.1f, 0.1f, 0.1f, 0.9f);
        }

        CardBorder->SetBorderBackgroundColor(BorderColor);
        SetRenderTransform(FSlateRenderTransform(Scale));
    }
}

FReply SUpgradeCard::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        if (OnCardClicked.IsBound())
        {
            OnCardClicked.Execute(CardIndex);
        }
        return FReply::Handled();
    }
    
    return FReply::Unhandled();
}

void SUpgradeCard::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    bIsHovered = true;
    UpdateVisualState();
}

void SUpgradeCard::OnMouseLeave(const FPointerEvent& MouseEvent)
{
    bIsHovered = false;
    UpdateVisualState();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
