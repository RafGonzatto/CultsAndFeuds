#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameMode.generated.h"

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

	// Cria e injeta o menu (chamado via timer no BeginPlay)
	void SpawnMenu();
};
