#pragma once
#include "Components/ActorComponent.h"
#include "SwarmProjectilePool.generated.h"


namespace SwarmCore { struct ProjectileRing; }
class UInstancedStaticMeshComponent;


UCLASS(ClassGroup = (Swarm), meta = (BlueprintSpawnableComponent))
class VAZIO_API USwarmProjectilePool : public UActorComponent {
	GENERATED_BODY()
public:
	void Init(UInstancedStaticMeshComponent* ISM) { ProjectileISM = ISM; }
	void SyncFromCore(const SwarmCore::ProjectileRing& R);
private: UPROPERTY() UInstancedStaticMeshComponent* ProjectileISM = nullptr;
};

