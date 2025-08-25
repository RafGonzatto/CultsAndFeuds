#include "Core/Flow/FlowSubsystem.h"
#include "Kismet/GameplayStatics.h"

bool UFlowSubsystem::OpenLevelByName(FName MapName)
{
	if (!GetWorld() || MapName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Flow] OpenLevelByName falhou (MapName vazio ou World nulo)"));
		return false;
	}
	UE_LOG(LogTemp, Warning, TEXT("[Flow] Abrindo mapa: %s"), *MapName.ToString());
	UGameplayStatics::OpenLevel(GetWorld(), MapName);
	return true;
}

bool UFlowSubsystem::OpenMainMenu()
{
	return OpenLevelByName(MainMenuMap);
}

bool UFlowSubsystem::OpenCity()
{
	return OpenLevelByName(CityMap);
}
