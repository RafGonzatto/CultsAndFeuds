#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Swarm/Upgrades/SwarmUpgradeSystem.h"

DECLARE_DELEGATE_OneParam(FOnUpgradeChosen, ESwarmUpgradeType);

class VAZIO_API SLevelUpModal : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLevelUpModal) {}
        SLATE_EVENT(FOnUpgradeChosen, OnUpgradeChosen)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void SetupUpgrades(const TArray<FSwarmUpgrade>& Upgrades);

    // SWidget interface
    virtual bool SupportsKeyboardFocus() const override { return true; }
    virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
    virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override;

private:
    FOnUpgradeChosen OnUpgradeChosen;
    TArray<FSwarmUpgrade> CurrentUpgrades;
    int32 SelectedIndex = 0;
    TArray<TSharedPtr<class SUpgradeCard>> UpgradeCards;
    TSharedPtr<class SHorizontalBox> CardsContainer;

    void SelectCard(int32 Index);
    void ConfirmSelection();
    void OnCardClicked(int32 CardIndex);
};
