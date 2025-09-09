#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Swarm/Upgrades/SwarmUpgradeSystem.h"
#include "SwarmUpgradeCard.generated.h"

DECLARE_DELEGATE_OneParam(FOnCardClicked, ESwarmUpgradeType);

UCLASS()
class VAZIO_API USwarmUpgradeCard : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    void SetupCard(const FSwarmUpgrade& Upgrade);
    void SetSelected(bool bIsSelected);

    FOnCardClicked OnCardClicked;

protected:
    UPROPERTY()
    class UBorder* CardBorder;

    UPROPERTY()
    class UBorder* IconBorder;

    UPROPERTY()
    class UTextBlock* TitleText;

    UPROPERTY()
    class UTextBlock* DescriptionText;

    void CreateCardStructure();

private:
    FSwarmUpgrade CardUpgrade;
    bool bIsHovered = false;
    bool bIsSelected = false;
};