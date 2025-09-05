#pragma once
#include "Components/ActorComponent.h"
#include "SwarmSpawnerComponent.generated.h"


class USwarmConfig; namespace SwarmCore { struct World; }


UCLASS(ClassGroup = (Swarm), meta = (BlueprintSpawnableComponent))
class VAZIO_API USwarmSpawnerComponent : public UActorComponent {
	GENERATED_BODY()
public: void SpawnInitial(SwarmCore::World& W, const USwarmConfig& Cfg, int32 PerType, int32 Seed);
};

