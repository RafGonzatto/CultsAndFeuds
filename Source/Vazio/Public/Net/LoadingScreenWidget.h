#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/CircularThrobber.h"
#include "LoadingScreenWidget.generated.h"

UCLASS()
class VAZIO_API ULoadingScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	ULoadingScreenWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable)
	void SetLoadingText(const FString& Text);
	
	UFUNCTION(BlueprintCallable)
	void ShowLoadingScreen();
	
	UFUNCTION(BlueprintCallable)
	void HideLoadingScreen();
	
	FText GetLoadingText() const;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	TSharedPtr<STextBlock> LoadingTextBlock;
	TSharedPtr<class SThrobber> LoadingSpinner;
	
	FString CurrentLoadingText;
};
