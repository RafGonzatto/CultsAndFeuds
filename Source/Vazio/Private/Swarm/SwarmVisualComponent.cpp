#include "Swarm/SwarmVisualComponent.h"
#include "Swarm/SwarmConfig.h"
#include "Components/InstancedStaticMeshComponent.h"


static UStaticMesh* ResolveMesh(const USwarmConfig& Cfg, const FSwarmEnemyTypeConfig& T) {
    if (T.OverrideMesh.ToSoftObjectPath().IsValid()) {
        return T.OverrideMesh.LoadSynchronous();
    }
    switch (T.Shape) {
        case ESwarmShape::Cube:    return Cfg.DefaultCube.LoadSynchronous();
        case ESwarmShape::Capsule: return Cfg.DefaultCapsule.LoadSynchronous();
        default:                   return Cfg.DefaultSphere.LoadSynchronous();
    }
}

static FVector ComputeUniformScale(UStaticMesh* Mesh, float TargetRadius)
{
    if (!Mesh || TargetRadius <= 0.f) return FVector(1.f);
    const FBoxSphereBounds B = Mesh->GetBounds();
    const float MeshRadius = B.SphereRadius > KINDA_SMALL_NUMBER ? B.SphereRadius : 50.f;
    const float S = TargetRadius / MeshRadius;
    return FVector(S);
}

void USwarmVisualComponent::BuildFromConfig(const USwarmConfig& Cfg) {
    for (auto* H : HISMs) if (H) H->DestroyComponent();
    HISMs.Reset();
    TypeScales.Reset();
    if (!GetOwner()) return;
    if (Cfg.EnemyTypes.Num() == 0) { UE_LOG(LogSwarm, Warning, TEXT("SwarmVisual: No EnemyTypes in Config; visuals will be empty.")); }
    for (int i = 0; i < Cfg.EnemyTypes.Num(); ++i) {
        auto* H = NewObject<UHierarchicalInstancedStaticMeshComponent>(GetOwner());
        H->SetupAttachment(GetOwner()->GetRootComponent());
        H->SetMobility(EComponentMobility::Movable);

    // Ajuste de colisão: inimigos do enxame NÃO devem empurrar / levantar o player.
    // Passam "através" e o dano é calculado manualmente via distância.
    H->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    H->SetCollisionObjectType(ECC_WorldDynamic);
    H->SetCollisionResponseToAllChannels(ECR_Ignore);
    // (Opcional) permitir overlaps com Pawn caso futuramente se queira efeitos de overlap
    H->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    H->SetGenerateOverlapEvents(true); // Mantém possibilidade de futuras detecções sem bloqueio

        UStaticMesh* M = ResolveMesh(Cfg, Cfg.EnemyTypes[i]);
        if (M) { H->SetStaticMesh(M); }
        else { UE_LOG(LogSwarm, Warning, TEXT("SwarmVisual: Type %d has no resolved StaticMesh (set DefaultSphere/Cube/Capsule or OverrideMesh)."), i); }
        H->NumCustomDataFloats = 4;
        H->RegisterComponent();
        HISMs.Add(H);
        // Compute desired uniform scale so that mesh visual radius matches config Radius
        const FVector S = ComputeUniformScale(M, Cfg.EnemyTypes[i].Radius);
        TypeScales.Add(S);
    }
}

void USwarmVisualComponent::UpdateTypeTransforms(int32 TypeIdx, const TArray<FTransform>& XfIn) {
    if (!HISMs.IsValidIndex(TypeIdx)) return;
    auto* H = HISMs[TypeIdx];
    const FVector Scale = TypeScales.IsValidIndex(TypeIdx) ? TypeScales[TypeIdx] : FVector(1.f);

    // Apply uniform scale per instance
    TArray<FTransform> Xf; Xf.Reserve(XfIn.Num());
    for (const FTransform& T : XfIn) {
        FTransform T2 = T; T2.SetScale3D(Scale); Xf.Add(T2);
    }

    const int need = Xf.Num();
    const int have = H->GetInstanceCount();
    if (have != need) {
        H->ClearInstances();
        if (need > 0) {
            H->AddInstances(Xf, false, true, true);
        }
        return;
    }
    if (need > 0) {
        H->BatchUpdateInstancesTransforms(0, Xf, false, true, true);
    }
}

UInstancedStaticMeshComponent* USwarmVisualComponent::EnsureProjectileISM(UStaticMesh* M) { if (!ProjectileISM) { ProjectileISM = NewObject<UInstancedStaticMeshComponent>(GetOwner()); ProjectileISM->SetupAttachment(GetOwner()->GetRootComponent()); ProjectileISM->RegisterComponent(); } if (M) ProjectileISM->SetStaticMesh(M); return ProjectileISM; }

