#include "Core/Flow/FlowSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

void UFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Warning, TEXT("[FlowSubsystem] Inicializado com mapas:"));
	UE_LOG(LogTemp, Warning, TEXT("  - MainMenu: %s"), *MainMenuMap.ToString());
	UE_LOG(LogTemp, Warning, TEXT("  - City: %s"), *CityMap.ToString());
	UE_LOG(LogTemp, Warning, TEXT("  - Battle: %s"), *BattleMap.ToString());

	// Debug na tela também
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Blue, 
			FString::Printf(TEXT("FlowSubsystem inicializado - City: %s"), *CityMap.ToString()));
	}
}

bool UFlowSubsystem::OpenLevelByName(FName MapName)
{
	if (!GetWorld() || MapName.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("[Flow] OpenLevelByName falhou (MapName vazio ou World nulo)"));
		return false;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[Flow] *** ABRINDO MAPA: %s ***"), *MapName.ToString());
	
	// Debug na tela
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
			FString::Printf(TEXT("Carregando mapa: %s"), *MapName.ToString()));
	}
	
	UGameplayStatics::OpenLevel(GetWorld(), MapName);
	return true;
}

bool UFlowSubsystem::OpenMainMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("[Flow] OpenMainMenu() chamado"));
	return OpenLevelByName(MainMenuMap);
}

bool UFlowSubsystem::OpenCity()
{
	UE_LOG(LogTemp, Warning, TEXT("[Flow] *** OpenCity() chamado - indo para %s ***"), *CityMap.ToString());
	
	// Debug na tela antes de trocar
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
			TEXT("*** INDO PARA A CIDADE ***"));
	}
	
	return OpenLevelByName(CityMap);
}

bool UFlowSubsystem::OpenBattle()
{
	UE_LOG(LogTemp, Warning, TEXT("[Flow] OpenBattle() chamado"));
	return OpenLevelByName(BattleMap);
}
