
#include "Swarm/Upgrades/SwarmUpgradeCard.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Blueprint/WidgetTree.h"

void USwarmUpgradeCard::NativeConstruct()
{
    Super::NativeConstruct();
    CreateCardStructure();
}

void USwarmUpgradeCard::CreateCardStructure()
{
    // Card border
    CardBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
    CardBorder->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.9f));
    CardBorder->SetPadding(FMargin(20));
    WidgetTree->RootWidget = CardBorder;

    // Vertical box for content
    UVerticalBox* ContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
    CardBorder->SetContent(ContentBox);

    // Icon container size box (to enforce consistent size)
    USizeBox* IconSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
    IconSizeBox->SetWidthOverride(80.f);
    IconSizeBox->SetHeightOverride(80.f);

    // Icon
    IconBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
    IconBorder->SetBrushColor(FLinearColor::White);
    IconSizeBox->AddChild(IconBorder);

    UVerticalBoxSlot* IconSlot = ContentBox->AddChildToVerticalBox(IconSizeBox);
    IconSlot->SetHorizontalAlignment(HAlign_Center);
    IconSlot->SetPadding(FMargin(0, 0, 0, 20));

    // Title
    TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    FSlateFontInfo TitleFont; TitleFont.Size = 24; TitleText->SetFont(TitleFont);
    TitleText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
    UVerticalBoxSlot* TitleSlot = ContentBox->AddChildToVerticalBox(TitleText);
    TitleSlot->SetHorizontalAlignment(HAlign_Center);
    TitleSlot->SetPadding(FMargin(0, 0, 0, 10));

    // Description
    DescriptionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    FSlateFontInfo DescFont; DescFont.Size = 14; DescriptionText->SetFont(DescFont);
    DescriptionText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    DescriptionText->SetAutoWrapText(true);
    UVerticalBoxSlot* DescSlot = ContentBox->AddChildToVerticalBox(DescriptionText);
    DescSlot->SetHorizontalAlignment(HAlign_Center);

    // Set size
    SetDesiredSizeInViewport(FVector2D(250, 350));
}

void USwarmUpgradeCard::SetupCard(const FSwarmUpgrade& Upgrade)
{
    CardUpgrade = Upgrade;

    if (TitleText)
    {
        TitleText->SetText(FText::FromString(Upgrade.Title));
    }
    if (DescriptionText)
    {
        DescriptionText->SetText(FText::FromString(Upgrade.Description));
    }
    if (IconBorder)
    {
        IconBorder->SetBrushColor(Upgrade.IconColor);
        FSlateBrush Brush; Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
        Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
        Brush.OutlineSettings.CornerRadii = FVector4(40,40,40,40);
        IconBorder->SetBrush(Brush);
    }
}

void USwarmUpgradeCard::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
    bIsHovered = true;
    if (CardBorder)
    {
        CardBorder->SetBrushColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f));
        SetRenderScale(FVector2D(1.05f, 1.05f));
    }
}

void USwarmUpgradeCard::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);
    bIsHovered = false;
    if (CardBorder && !bIsSelected)
    {
        CardBorder->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.9f));
        SetRenderScale(FVector2D(1.0f, 1.0f));
    }
}

FReply USwarmUpgradeCard::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        if (OnCardClicked.IsBound())
        {
            OnCardClicked.Execute(CardUpgrade.Type);
        }
        return FReply::Handled();
    }
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void USwarmUpgradeCard::SetSelected(bool bInSelected)
{
    bIsSelected = bInSelected;
    if (CardBorder)
    {
        if (bIsSelected)
        {
            CardBorder->SetBrushColor(FLinearColor(0.3f, 0.3f, 0.1f, 1.0f));
            SetRenderScale(FVector2D(1.1f, 1.1f));
        }
        else
        {
            CardBorder->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.9f));
            SetRenderScale(FVector2D(1.0f, 1.0f));
        }
    }
}
