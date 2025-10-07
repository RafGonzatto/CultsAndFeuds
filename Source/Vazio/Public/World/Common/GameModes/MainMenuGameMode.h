#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameMode.generated.h"

// Forward declaration to avoid needing the full widget header in this public header
class UMainMenuWidget;

UCLASS()
class VAZIO_API AMainMenuGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMainMenuGameMode();

protected:
    virtual void BeginPlay() override;

private:
	UPROPERTY()
        UMainMenuWidget* Menu = nullptr;
    void CreateMenuCamera();
};