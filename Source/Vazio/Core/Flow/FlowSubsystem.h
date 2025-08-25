#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FlowSubsystem.generated.h"

UENUM(BlueprintType)
enum class EVazioMode : uint8
{
	MainMenu,
	City,
	Battle
};

UCLASS(Config=Game)
class VAZIO_API UFlowSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Ajuste os nomes dos mapas conforme voc� criar no Content
	UPROPERTY(Config)
	FName MainMenuMap = TEXT("MainMenu");

	UPROPERTY(Config)
	FName CityMap = TEXT("City_P");

	// Carrega por nome simples (os dois mapas devem estar em /Game/Maps/.. com esses nomes)
	UFUNCTION()
	bool OpenLevelByName(FName MapName);

	// Atalhos
	UFUNCTION() bool OpenMainMenu();
	UFUNCTION() bool OpenCity();
};
