#include "UI/LevelUp/SLevelUpModal.h"
#include "UI/LevelUp/SUpgradeCard.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLevelUpModal::Construct(const FArguments& InArgs)
{
    OnUpgradeChosen = InArgs._OnUpgradeChosen;

    ChildSlot
    [
        SNew(SOverlay)
        
        // Dark backdrop
        + SOverlay::Slot()
        [
            SNew(SBorder)
            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.8f))
            .Padding(0)
        ]
        
        // Content layer
        + SOverlay::Slot()
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        [
            SNew(SVerticalBox)
            
            // Title
            + SVerticalBox::Slot()
            .AutoHeight()
            .HAlign(HAlign_Center)
            .Padding(0, 0, 0, 20)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("LEVEL UP!")))
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 48))
                .ColorAndOpacity(FLinearColor::Yellow)
                .Justification(ETextJustify::Center)
            ]
            
            // Subtitle
            + SVerticalBox::Slot()
            .AutoHeight()
            .HAlign(HAlign_Center)
            .Padding(0, 0, 0, 40)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Escolha um upgrade")))
                .Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
                .ColorAndOpacity(FLinearColor::White)
                .Justification(ETextJustify::Center)
            ]
            
            // Cards container
            + SVerticalBox::Slot()
            .AutoHeight()
            .HAlign(HAlign_Center)
            [
                SAssignNew(CardsContainer, SHorizontalBox)
            ]
        ]
    ];

    UE_LOG(LogTemp, Warning, TEXT("LevelUp:UI:SlateModalConstructed"));
}

void SLevelUpModal::SetupUpgrades(const TArray<FSwarmUpgrade>& Upgrades)
{
    CurrentUpgrades = Upgrades;
    UpgradeCards.Empty();
    CardsContainer->ClearChildren();

    for (int32 i = 0; i < Upgrades.Num(); i++)
    {
        TSharedPtr<SUpgradeCard> Card = SNew(SUpgradeCard)
            .Upgrade(Upgrades[i])
            .CardIndex(i);

        Card->OnCardClicked.BindSP(this, &SLevelUpModal::OnCardClicked);
        UpgradeCards.Add(Card);

        CardsContainer->AddSlot()
        .AutoWidth()
        .Padding(20, 0)
        [
            Card.ToSharedRef()
        ];
    }

    // Select first card by default
    if (UpgradeCards.Num() > 0)
    {
        SelectCard(0);
    }

    UE_LOG(LogTemp, Warning, TEXT("LevelUp:UI:CardsSetup Count=%d"), Upgrades.Num());
}

void SLevelUpModal::SelectCard(int32 Index)
{
    if (Index < 0 || Index >= UpgradeCards.Num())
    {
        return;
    }

    // Deselect previous
    if (SelectedIndex >= 0 && SelectedIndex < UpgradeCards.Num())
    {
        UpgradeCards[SelectedIndex]->SetSelected(false);
    }

    // Select new
    SelectedIndex = Index;
    UpgradeCards[SelectedIndex]->SetSelected(true);
    
    UE_LOG(LogTemp, Warning, TEXT("LevelUp:UI:CardSelected Index=%d"), Index);
}

void SLevelUpModal::ConfirmSelection()
{
    if (SelectedIndex >= 0 && SelectedIndex < CurrentUpgrades.Num())
    {
        ESwarmUpgradeType SelectedType = CurrentUpgrades[SelectedIndex].Type;
        UE_LOG(LogTemp, Warning, TEXT("LevelUp:UI:ConfirmSelection Type=%d"), (int32)SelectedType);
        
        if (OnUpgradeChosen.IsBound())
        {
            OnUpgradeChosen.Execute(SelectedType);
        }
    }
}

void SLevelUpModal::OnCardClicked(int32 CardIndex)
{
    SelectCard(CardIndex);
    ConfirmSelection();
}

FReply SLevelUpModal::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
    FKey Key = InKeyEvent.GetKey();

    if (Key == EKeys::Left || Key == EKeys::A)
    {
        SelectCard((SelectedIndex - 1 + UpgradeCards.Num()) % UpgradeCards.Num());
        return FReply::Handled();
    }
    else if (Key == EKeys::Right || Key == EKeys::D)
    {
        SelectCard((SelectedIndex + 1) % UpgradeCards.Num());
        return FReply::Handled();
    }
    else if (Key == EKeys::Enter || Key == EKeys::SpaceBar)
    {
        ConfirmSelection();
        return FReply::Handled();
    }

    return FReply::Unhandled();
}

FReply SLevelUpModal::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent)
{
    UE_LOG(LogTemp, Warning, TEXT("LevelUp:UI:FocusReceived"));
    return SCompoundWidget::OnFocusReceived(MyGeometry, InFocusEvent);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
