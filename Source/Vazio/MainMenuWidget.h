// MainMenuWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UOverlay;
class UVerticalBox;

UCLASS()
class VAZIO_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMainMenuWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual TSharedRef<class SWidget> RebuildWidget() override;

private:
	// raiz e coluna de bot�es
	UPROPERTY() UOverlay* RootOverlay = nullptr;
	UPROPERTY() UVerticalBox* MenuBox = nullptr;

	// Handlers precisam ser UFUNCTION para AddDynamic
	UFUNCTION() void HandlePlay();
	UFUNCTION() void HandleContinue();
	UFUNCTION() void HandleSettings();
	UFUNCTION() void HandleExit();

	// helper para criar botão + bind por nome (dinâmico)
	void AddButton(const FText& Label, FName HandlerName);
};
