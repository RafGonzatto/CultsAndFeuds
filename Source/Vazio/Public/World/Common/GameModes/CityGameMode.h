// CityGameMode.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CityGameMode.generated.h"

class UStaticMesh;
class UMaterialInterface;

UCLASS()
class VAZIO_API ACityGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACityGameMode();

protected:
	virtual void BeginPlay() override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

private:
	// === Setup / Ambiente ===
	UFUNCTION() void CreatePlayerStartIfNeeded();
	UFUNCTION() void CreateEnvironmentNextTick();
	UFUNCTION() void CreateCityGround();
	UFUNCTION() void CreateReferenceCubes();
	UFUNCTION() void CreateBasicLighting();
	UFUNCTION() void CreateCityBounds();         // (mantido desabilitado)
	void CreateBoundaryWall(FVector Location, FVector Scale);

	// === Helpers ===
	class AStaticMeshActor* SpawnMeshActor(const FVector& Loc, const FRotator& Rot, const FVector& Scale) const;
	void ApplyColorMID(UPrimitiveComponent* Comp, const FLinearColor& Color) const;

private:
	// Assets carregados no construtor
	UPROPERTY() UStaticMesh* CubeMesh = nullptr;
	UPROPERTY() UMaterialInterface* BasicMaterial = nullptr;

	// Identificador do nosso spawn
	static FName GetCitySpawnTag() { return TEXT("CitySpawn"); }
};
