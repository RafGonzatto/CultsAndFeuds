#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Swarm/Upgrades/SwarmUpgradeSystem.h"
#include "SwarmLevelUpWidget.generated.h"

DECLARE_DELEGATE_OneParam(FOnUpgradeSelected, ESwarmUpgradeType);

UCLASS()
class VAZIO_API USwarmLevelUpWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    void SetupUpgrades(const TArray<FSwarmUpgrade>& Upgrades);
    FOnUpgradeSelected OnUpgradeSelected;

protected:
    UPROPERTY()
    class UCanvasPanel* RootCanvas;

    UPROPERTY()
    class UBorder* DarkOverlay;

    UPROPERTY()
    class UTextBlock* TitleText;

    UPROPERTY()
    class UTextBlock* SubtitleText;

    UPROPERTY()
    class UHorizontalBox* CardsContainer;

    UPROPERTY()
    TArray<class USwarmUpgradeCard*> UpgradeCards;

    void CreateWidgetStructure();
    void SelectCard(int32 Index);

private:
    int32 CurrentSelection = 0;
    TArray<FSwarmUpgrade> CurrentUpgrades;
};