#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuPlayerController.generated.h"

UCLASS()
class VAZIO_API AMainMenuPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    AMainMenuPlayerController();

protected:
    virtual void BeginPlay() override;
    UFUNCTION() void SpawnMenu();

private:
    UPROPERTY() UUserWidget* Menu;
    UPROPERTY(EditDefaultsOnly, Category="UI")
    TSubclassOf<UUserWidget> MenuClass; // defina no BP OU em C++
};
