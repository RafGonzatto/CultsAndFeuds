#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "FlowSubsystem.generated.h"

UENUM(BlueprintType)
enum class EVazioMode : uint8 { MainMenu, City, Battle };

UCLASS(Config=Game)
class VAZIO_API UFlowSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	// Use soft refs p/ garantir cozimento e validação em runtime
	UPROPERTY(Config, EditAnywhere, Category="Flow", meta=(AllowedClasses="World"))
	TSoftObjectPtr<UWorld> MainMenuMap;

	UPROPERTY(Config, EditAnywhere, Category="Flow", meta=(AllowedClasses="World"))
	TSoftObjectPtr<UWorld> CityMap;

	UPROPERTY(Config, EditAnywhere, Category="Flow", meta=(AllowedClasses="World"))
	TSoftObjectPtr<UWorld> BattleMap;

	UFUNCTION(BlueprintCallable) bool OpenLevelByRef(const TSoftObjectPtr<UWorld>& MapRef);
	UFUNCTION(BlueprintCallable) bool Open(EVazioMode Mode);

	// Atalhos
	UFUNCTION(BlueprintCallable) bool OpenMainMenu() { return Open(EVazioMode::MainMenu); }
	UFUNCTION(BlueprintCallable) bool OpenCity()     { return Open(EVazioMode::City); }
	UFUNCTION(BlueprintCallable) bool OpenBattle()   { return Open(EVazioMode::Battle); }

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	bool ResolveLevelName(const TSoftObjectPtr<UWorld>& MapRef, FName& OutLevelName) const;
	bool IsCurrentLevel(const FName& LevelName) const;
	void LogMap(const TCHAR* Label, const TSoftObjectPtr<UWorld>& MapRef) const;
};
