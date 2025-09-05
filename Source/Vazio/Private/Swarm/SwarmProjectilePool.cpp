#include "SwarmProjectilePool.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Swarm/Core/Projectile.h"
using namespace SwarmCore;
void USwarmProjectilePool::SyncFromCore(const ProjectileRing& R) {
	if (!ProjectileISM) return;
	int need = (int)R.Items.size();
	int have = ProjectileISM->GetInstanceCount();
	if (have < need) {
		TArray<FTransform> add;
		add.Reserve(need - have);
		for (int i = have; i < need; ++i) add.Add(FTransform::Identity);
		ProjectileISM->AddInstances(add, false, true, true);
	}
	TArray<FTransform> xf;
	xf.Reserve(need);
	for (int i = 0; i < need; ++i) {
		const auto& it = R.Items[i];
		if (it.alive) xf.Add(FTransform(FRotator::ZeroRotator, FVector(it.p.x, it.p.y, it.p.z)));
		else xf.Add(FTransform(FRotator::ZeroRotator, FVector(0, 0, -1e9)));
	}
	ProjectileISM->BatchUpdateInstancesTransforms(0, xf, false, true, true);
}