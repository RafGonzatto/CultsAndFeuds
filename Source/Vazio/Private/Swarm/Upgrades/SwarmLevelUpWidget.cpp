#include "Swarm/Upgrades/SwarmLevelUpWidget.h"
#include "Swarm/Upgrades/SwarmUpgradeCard.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Blueprint/WidgetTree.h"

#pragma warning(push)
#pragma warning(disable:4458)

void USwarmLevelUpWidget::NativeConstruct()
{
    Super::NativeConstruct();
    CreateWidgetStructure();
}

void USwarmLevelUpWidget::CreateWidgetStructure()
{
    // Create root canvas
    RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
    WidgetTree->RootWidget = RootCanvas;

    // Dark overlay
    DarkOverlay = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
    DarkOverlay->SetBrushColor(FLinearColor(0, 0, 0, 0.8f));
    
    UCanvasPanelSlot* OverlaySlot = RootCanvas->AddChildToCanvas(DarkOverlay);
    OverlaySlot->SetAnchors(FAnchors(0, 0, 1, 1));
    OverlaySlot->SetOffsets(FMargin(0));

    // Title
    TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    TitleText->SetText(FText::FromString(TEXT("LEVEL UP!")));
    FSlateFontInfo TitleFont;
    TitleFont.Size = 48;
    TitleText->SetFont(TitleFont);
    TitleText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
    
    UCanvasPanelSlot* TitleSlot = RootCanvas->AddChildToCanvas(TitleText);
    TitleSlot->SetAnchors(FAnchors(0.5f, 0.2f));
    TitleSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    TitleSlot->SetAutoSize(true);

    // Subtitle
    SubtitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    SubtitleText->SetText(FText::FromString(TEXT("Escolha um upgrade")));
    FSlateFontInfo SubtitleFont;
    SubtitleFont.Size = 24;
    SubtitleText->SetFont(SubtitleFont);
    SubtitleText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    
    UCanvasPanelSlot* SubtitleSlot = RootCanvas->AddChildToCanvas(SubtitleText);
    SubtitleSlot->SetAnchors(FAnchors(0.5f, 0.3f));
    SubtitleSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    SubtitleSlot->SetAutoSize(true);

    // Cards container
    CardsContainer = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
    
    UCanvasPanelSlot* CardsSlot = RootCanvas->AddChildToCanvas(CardsContainer);
    CardsSlot->SetAnchors(FAnchors(0.5f, 0.5f));
    CardsSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    CardsSlot->SetAutoSize(true);
}

void USwarmLevelUpWidget::SetupUpgrades(const TArray<FSwarmUpgrade>& Upgrades)
{
    CurrentUpgrades = Upgrades;
    UpgradeCards.Empty();

    if (!CardsContainer)
    {
        return;
    }

    CardsContainer->ClearChildren();

    for (int32 i = 0; i < Upgrades.Num(); i++)
    {
        USwarmUpgradeCard* Card = CreateWidget<USwarmUpgradeCard>(GetOwningPlayer(), USwarmUpgradeCard::StaticClass());
        if (Card)
        {
            Card->SetupCard(Upgrades[i]);
            Card->OnCardClicked.BindLambda([this, i](ESwarmUpgradeType SelectedUpgradeType)
            {
                if (OnUpgradeSelected.IsBound())
                {
                    OnUpgradeSelected.Execute(SelectedUpgradeType);
                }
            });

            UHorizontalBoxSlot* HBSlot = CardsContainer->AddChildToHorizontalBox(Card);
            HBSlot->SetPadding(FMargin(20, 0));

            UpgradeCards.Add(Card);
        }
    }

    // Select first card by default
    if (UpgradeCards.Num() > 0)
    {
        SelectCard(0);
    }
}

FReply USwarmLevelUpWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    FKey Key = InKeyEvent.GetKey();

    if (Key == EKeys::Left || Key == EKeys::A)
    {
        SelectCard((CurrentSelection - 1 + UpgradeCards.Num()) % UpgradeCards.Num());
        return FReply::Handled();
    }
    else if (Key == EKeys::Right || Key == EKeys::D)
    {
        SelectCard((CurrentSelection + 1) % UpgradeCards.Num());
        return FReply::Handled();
    }
    else if (Key == EKeys::Enter || Key == EKeys::SpaceBar)
    {
        if (CurrentSelection >= 0 && CurrentSelection < CurrentUpgrades.Num())
        {
            if (OnUpgradeSelected.IsBound())
            {
                OnUpgradeSelected.Execute(CurrentUpgrades[CurrentSelection].Type);
            }
        }
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void USwarmLevelUpWidget::SelectCard(int32 Index)
{
    if (Index < 0 || Index >= UpgradeCards.Num())
    {
        return;
    }

    // Deselect previous
    if (CurrentSelection >= 0 && CurrentSelection < UpgradeCards.Num())
    {
        UpgradeCards[CurrentSelection]->SetSelected(false);
    }

    // Select new
    CurrentSelection = Index;
    UpgradeCards[CurrentSelection]->SetSelected(true);
}

#pragma warning(pop)
