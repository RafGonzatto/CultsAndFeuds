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

	// Criar PlayerStart se n�o existir
	UFUNCTION()
	void CreatePlayerStartIfNeeded();
	
	// Criar ambiente com delay
	UFUNCTION()
	void CreateEnvironmentDelayed();
	
	// Criar o ch�o da cidade
	UFUNCTION()
	void CreateCityGround();
	
	// Criar cubos de refer�ncia coloridos
	UFUNCTION()
	void CreateReferenceCubes();
	
	// Criar ilumina��o b�sica
	UFUNCTION()
	void CreateBasicLighting();
	
	// Criar limites da cidade (paredes invis�veis) - desabilitado por enquanto
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
	
	// Timer para delay na cria��o do ambiente
	FTimerHandle SpawnDelayTimer;
};
