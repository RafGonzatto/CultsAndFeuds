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

	UFUNCTION()
	void HandleClose();

private:
    TSharedPtr<class SButton> CloseButton;
};
