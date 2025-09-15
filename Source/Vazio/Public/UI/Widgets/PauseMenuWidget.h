#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "PauseMenuWidget.generated.h"

/**
 * Widget de pausa principal com opções Continue, Settings, Exit
 */
UCLASS()
class VAZIO_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPauseMenuWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// UI Components
	UPROPERTY(meta = (BindWidget))
	class UButton* ContinueButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* SettingsButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* ExitButton;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TitleText;

	// Button Click Handlers
	UFUNCTION()
	void OnContinueClicked();

	UFUNCTION()
	void OnSettingsClicked();

	UFUNCTION()
	void OnExitClicked();

public:
	// Static function to show/hide pause menu
	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	static void ShowPauseMenu(UWorld* World);

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	static void HidePauseMenu(UWorld* World);

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	static bool IsPauseMenuVisible(UWorld* World);

private:
	// Static instance tracking
	static TWeakObjectPtr<UPauseMenuWidget> CurrentPauseMenu;
};