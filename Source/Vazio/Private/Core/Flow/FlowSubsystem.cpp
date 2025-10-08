#include "Core/Flow/FlowSubsystem.h"
#include "Logging/VazioLogFacade.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

void UFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LOG_SYSTEMS(Info, TEXT("[Flow] Inicializado"));
	LogMap(TEXT("MainMenu"), MainMenuMap);
	LogMap(TEXT("City"),     CityMap);
	LogMap(TEXT("Battle"),   BattleMap);
}

void UFlowSubsystem::LogMap(const TCHAR* Label, const TSoftObjectPtr<UWorld>& MapRef) const
{
	const FString Path = MapRef.IsNull() ? TEXT("<vazio>") : MapRef.ToSoftObjectPath().ToString();
	LOG_SYSTEMS(Info, TEXT("  - %s: %s"), Label, *Path);
}

bool UFlowSubsystem::ResolveLevelName(const TSoftObjectPtr<UWorld>& MapRef, FName& OutLevelName) const
{
	if (MapRef.IsNull())
	{
		LOG_SYSTEMS(Error, TEXT("[Flow] Mapa não configurado (soft reference vazia)."));
		return false;
	}
	const FString LongName = MapRef.GetLongPackageName(); // ex: /Game/Levels/City_Main
	if (LongName.IsEmpty())
	{
		LOG_SYSTEMS(Error, TEXT("[Flow] Soft reference inválida: %s"), *MapRef.ToString());
		return false;
	}
	OutLevelName = FName(*LongName);
	return true;
}

bool UFlowSubsystem::IsCurrentLevel(const FName& LevelName) const
{
	if (const UWorld* World = GetWorld())
	{
		// GetMapName() retorna "UEDPIE_0_Map" em PIE; checamos por sufixo para tolerar prefixos
		return World->GetMapName().EndsWith(LevelName.ToString(), ESearchCase::IgnoreCase);
	}
	return false;
}

bool UFlowSubsystem::OpenLevelByRef(const TSoftObjectPtr<UWorld>& MapRef)
{
	if (!GetWorld())
	{
		LOG_SYSTEMS(Error, TEXT("[Flow] World nulo."));
		return false;
	}

	FName LevelName;
	if (!ResolveLevelName(MapRef, LevelName)) return false;

	if (IsCurrentLevel(LevelName))
	{
		LOG_SYSTEMS(Debug, TEXT("[Flow] Já estamos no mapa: %s (ignorando)"), *LevelName.ToString());
		return true;
	}

	LOG_SYSTEMS(Info, TEXT("[Flow] Abrindo mapa: %s"), *LevelName.ToString());
	// Nota: OpenLevel inicia a viagem; sucesso real só é certo após carregar.
	UGameplayStatics::OpenLevel(this, LevelName);
	// OpenLevel returns void, assume success
	{
	}
	return true;
}

bool UFlowSubsystem::Open(EVazioMode Mode)
{
	switch (Mode)
	{
		case EVazioMode::MainMenu: return OpenLevelByRef(MainMenuMap);
		case EVazioMode::City:     return OpenLevelByRef(CityMap);
		case EVazioMode::Battle:   return OpenLevelByRef(BattleMap);
		default:
			LOG_SYSTEMS(Error, TEXT("[Flow] Mode inválido."));
			return false;
	}
}
