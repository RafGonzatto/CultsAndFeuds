#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldPlacerConfig.h"
#include "WorldPlacer.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class UStaticMeshComponent;
class UInteractableComponent;

UCLASS()
class VAZIO_API AWorldPlacer : public AActor
{
	GENERATED_BODY()

public:
	AWorldPlacer();

	UPROPERTY(EditAnywhere, Category="Placement")
	TObjectPtr<UWorldPlacerConfig> Config;

	// Debug
	UPROPERTY(EditAnywhere, Category="Placement|Debug")
	bool bDebugDrawPlacement = true;

	UPROPERTY(EditAnywhere, Category="Placement|Debug", meta=(EditCondition="bDebugDrawPlacement", ClampMin="0.0"))
	float DebugDrawTime = 10.0f;

	// Ajustes automáticos
	UPROPERTY(EditAnywhere, Category="Placement|Adjust")
	bool bPlaceOnGround = true;
	// Alturas de traçado em centímetros (world space)
	UPROPERTY(EditAnywhere, Category="Placement|Adjust", meta=(ClampMin="0.0"))
	float GroundTraceUp = 1000.f;

	UPROPERTY(EditAnywhere, Category="Placement|Adjust", meta=(ClampMin="0.0"))
	float GroundTraceDown = 5000.f;
	// Offset adicional após encostar no chão (cm)
	UPROPERTY(EditAnywhere, Category="Placement|Adjust")
	float GroundOffset = 0.f;

	// Alinhar a BASE do mesh ao chão (e não o pivô). Usa bounds para calcular o deslocamento.
	UPROPERTY(EditAnywhere, Category="Placement|Adjust")
	bool bAlignBaseToGround = true;

	// Normalização de altura do mesh (cm). Útil para corrigir import com escala errada.
	UPROPERTY(EditAnywhere, Category="Placement|Adjust")
	bool bNormalizeHeight = false;

	UPROPERTY(EditAnywhere, Category="Placement|Adjust", meta=(EditCondition="bNormalizeHeight", ClampMin="1.0"))
	float TargetHeight = 200.f;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

private:
	// Pools por Mesh
	UPROPERTY(Transient)
	TMap<TObjectPtr<UStaticMesh>, TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> HismPools;

	// Componentes unitários criados
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> SingleMeshComponents;

	// Componentes de interação criados (para limpar corretamente em rebuild)
	UPROPERTY(Transient)
	TArray<TObjectPtr<UInteractableComponent>> InteractableComponents;

	void ClearBuilt();
	void Build(bool bInEditor);

	UHierarchicalInstancedStaticMeshComponent* GetOrCreateHISM(UStaticMesh* Mesh, bool bMovable, const FName& CollisionProfile);
	UStaticMeshComponent* CreateSingleMeshComponent(const FWorldPlaceEntry& Entry);
	void AddInteractableToComponent(USceneComponent* AttachTo, const FWorldPlaceEntry& Entry);
};

