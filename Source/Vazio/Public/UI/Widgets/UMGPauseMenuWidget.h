#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UMGPauseMenuWidget.generated.h"

/**
 * UMG version of pause menu widget for Blueprint usage
 */
UCLASS(BlueprintType, Blueprintable)
class VAZIO_API UUMGPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UUMGPauseMenuWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// UI Components (will be bound in Blueprint)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* ContinueButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* SettingsButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* ExitButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TitleText;

	// Button Click Handlers
	UFUNCTION(BlueprintCallable)
	void OnContinueClicked();

	UFUNCTION(BlueprintCallable)
	void OnSettingsClicked();

	UFUNCTION(BlueprintCallable)
	void OnExitClicked();

public:
	// Static functions for pause menu managementP
	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	static void ShowPauseMenu(UWorld* World);

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	static void HidePauseMenu(UWorld* World);

	UFUNCTION(BlueprintPure, Category = "Pause Menu")
	static bool IsPauseMenuVisible(UWorld* World);

private:
	// Static instance tracking
	static TWeakObjectPtr<UUMGPauseMenuWidget> CurrentPauseMenu;
};