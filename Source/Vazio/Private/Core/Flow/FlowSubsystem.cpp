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
}

bool UFlowSubsystem::OpenLevelByName(FName MapName)
{
    if (!GetWorld() || MapName.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("[Flow] OpenLevelByName falhou (MapName vazio ou World nulo)"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Flow] *** ABRINDO MAPA: %s ***"), *MapName.ToString());
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
    return OpenLevelByName(CityMap);
}

bool UFlowSubsystem::OpenBattle()
{
    UE_LOG(LogTemp, Warning, TEXT("[Flow] OpenBattle() chamado"));
    return OpenLevelByName(BattleMap);
}

