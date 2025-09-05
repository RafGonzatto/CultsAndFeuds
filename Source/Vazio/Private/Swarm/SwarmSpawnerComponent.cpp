#include "Swarm/SwarmSpawnerComponent.h"
#include "Swarm/SwarmConfig.h"
#include <random>
#include "Swarm/Core/World.h"
using namespace SwarmCore;
void USwarmSpawnerComponent::SpawnInitial(World& W, const USwarmConfig& Cfg, int32 PerType, int32 Seed) { std::mt19937 rng(Seed); std::uniform_real_distribution<float> d(-Cfg.WorldExtent, Cfg.WorldExtent); for (int t = 0; t < Cfg.EnemyTypes.Num(); ++t) for (int i = 0; i < PerType; ++i) W.Spawn((type_t)t, d(rng), d(rng), d(rng)); }

