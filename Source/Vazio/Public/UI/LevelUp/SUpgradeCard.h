#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Gameplay/Upgrades/UpgradeSystem.h"

class VAZIO_API SUpgradeCard : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SUpgradeCard) {}
        SLATE_ARGUMENT(FUpgradeData, Upgrade)
        SLATE_ARGUMENT(int32, CardIndex)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void SetSelected(bool bSelected);

    // SWidget interface
    virtual bool SupportsKeyboardFocus() const override { return true; }
    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

    DECLARE_DELEGATE_OneParam(FOnCardClicked, int32);
    FOnCardClicked OnCardClicked;

private:
    FUpgradeData CardUpgrade;
    int32 CardIndex = 0;
    bool bIsSelected = false;
    bool bIsHovered = false;

    TSharedPtr<class SBorder> CardBorder;
    TSharedPtr<class SBorder> IconBorder;
    TSharedPtr<class STextBlock> TitleText;
    TSharedPtr<class STextBlock> DescriptionText;

    void UpdateVisualState();
};
