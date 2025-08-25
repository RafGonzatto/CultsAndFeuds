#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CityGameMode.generated.h"

class UStaticMesh;
class UMaterial;

UCLASS()
class VAZIO_API ACityGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACityGameMode();

protected:
	virtual void BeginPlay() override;
	
	// Override para escolher PlayerStart correto
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// Criar PlayerStart se não existir
	UFUNCTION()
	void CreatePlayerStartIfNeeded();
	
	// Criar ambiente com delay
	UFUNCTION()
	void CreateEnvironmentDelayed();
	
	// Criar o chão da cidade
	UFUNCTION()
	void CreateCityGround();
	
	// Criar cubos de referência coloridos
	UFUNCTION()
	void CreateReferenceCubes();
	
	// Criar iluminação básica
	UFUNCTION()
	void CreateBasicLighting();
	
	// Criar limites da cidade (paredes invisíveis) - desabilitado por enquanto
	UFUNCTION()
	void CreateCityBounds();
	
	// Helper para criar uma parede limite
	void CreateBoundaryWall(FVector Location, FVector Scale);

private:
	// Assets carregados no constructor
	UPROPERTY()
	UStaticMesh* CubeMesh = nullptr;
	
	UPROPERTY()
	UMaterial* BasicMaterial = nullptr;
	
	// Timer para delay na criação do ambiente
	FTimerHandle SpawnDelayTimer;
};
