#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UOverlay;
class UVerticalBox;
class UButton;
class UTextBlock;
class UWidgetTree;

UCLASS()
class VAZIO_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandlePlay();

private:
	// Guardas (não UPROPERTY pq criamos em runtime e o WidgetTree já mantém referências)
	UOverlay* RootOverlay = nullptr;
	UVerticalBox* MenuBox = nullptr;

	UButton* CreatePlayButton();
};
