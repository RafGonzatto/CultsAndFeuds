#pragma once
#include "Components/ActorComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "SwarmVisualComponent.generated.h"


class USwarmConfig;


UCLASS(ClassGroup = (Swarm), meta = (BlueprintSpawnableComponent))
class VAZIO_API USwarmVisualComponent : public UActorComponent {
	GENERATED_BODY()
public:
	void BuildFromConfig(const USwarmConfig& Cfg);
	void UpdateTypeTransforms(int32 TypeIdx, const TArray<FTransform>& Xf);
	UInstancedStaticMeshComponent* EnsureProjectileISM(UStaticMesh* M);
private:
	UPROPERTY() TArray<UHierarchicalInstancedStaticMeshComponent*> HISMs;
	UPROPERTY() UInstancedStaticMeshComponent* ProjectileISM = nullptr;
	// Per-type uniform scale computed from mesh bounds and configured Radius
	UPROPERTY() TArray<FVector> TypeScales;
};