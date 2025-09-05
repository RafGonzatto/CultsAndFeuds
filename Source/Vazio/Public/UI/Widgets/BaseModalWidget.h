#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseModalWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnModalClosed);

UCLASS()
class VAZIO_API UBaseModalWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    UBaseModalWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(BlueprintAssignable, Category="Modal")
	FOnModalClosed OnModalClosed;

	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category="Modal")
	void FocusFirstWidget();

protected:
    // Fecha ao pressionar E ou Escape
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
    virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    // Hooks para conteúdo customizado por modal
    virtual FText GetTitle() const;
    virtual TSharedRef<SWidget> BuildBody();

	UFUNCTION()
	void HandleClose();

private:
    TSharedPtr<class SButton> CloseButton;
};

// House Modal
UCLASS()
class VAZIO_API UHouseModalWidget : public UBaseModalWidget
{
    GENERATED_BODY()
protected:
    virtual FText GetTitle() const override { return FText::FromString(TEXT("House")); }
    virtual TSharedRef<SWidget> BuildBody() override;
};

// Arena Modal
UCLASS()
class VAZIO_API UArenaModalWidget : public UBaseModalWidget
{
    GENERATED_BODY()
protected:
    virtual FText GetTitle() const override { return FText::FromString(TEXT("Arena")); }
    virtual TSharedRef<SWidget> BuildBody() override;
};

// Shop Modal
UCLASS()
class VAZIO_API UShopModalWidget : public UBaseModalWidget
{
    GENERATED_BODY()
protected:
    virtual FText GetTitle() const override { return FText::FromString(TEXT("Shop")); }
    virtual TSharedRef<SWidget> BuildBody() override;
};