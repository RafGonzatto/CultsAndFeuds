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
	// Nomes dos mapas corrigidos para os mapas que você criou
	UPROPERTY(Config)
	FName MainMenuMap = TEXT("MainMenu");

	UPROPERTY(Config)
	FName CityMap = TEXT("City_Main");  // Corrigido para o nome do seu mapa

	UPROPERTY(Config)
	FName BattleMap = TEXT("Battle_Main");  // Adicionado o mapa de batalha também

	// Carrega por nome simples (os mapas devem estar em /Game/Levels/...)
	UFUNCTION()
	bool OpenLevelByName(FName MapName);

	// Atalhos
	UFUNCTION() bool OpenMainMenu();
	UFUNCTION() bool OpenCity();
	UFUNCTION() bool OpenBattle();

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
};
