#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Overlay.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Blueprint/WidgetTree.h"
#include "MainMenuWidget.generated.h"

class UWidget;

UCLASS()
class VAZIO_API UMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UMainMenuWidget(const FObjectInitializer& ObjectInitializer);
    UFUNCTION(BlueprintCallable, Category="MainMenu")
    UWidget* GetInitialFocusTarget() const;

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;

    UFUNCTION() void HandlePlay();

private:
    UPROPERTY() UOverlay* RootOverlay;
    UPROPERTY() UVerticalBox* MenuBox;
    UPROPERTY() UButton* PlayButton;

    UButton* CreatePlayButton();
};
