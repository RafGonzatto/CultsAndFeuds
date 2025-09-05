#include "Swarm/SwarmVisibilityComponent.h"
#include "Swarm/SwarmConfig.h"
#include "Swarm/Core/World.h"
#include "Async/ParallelFor.h"
#include "HAL/CriticalSection.h"

USwarmVisibilityComponent::USwarmVisibilityComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USwarmVisibilityComponent::Compute(const SwarmCore::World& W, const FVector& CamPos, const FVector& CamForward, float FOVAngle, const USwarmConfig& Cfg)
{
    const int32 Types = Cfg.EnemyTypes.Num();
    VisibleByType.SetNum(Types);
    for (int32 t = 0; t < Types; ++t) VisibleByType[t].Reset();
    VisibleTotal = 0;

    const FVector Fwd = CamForward.GetSafeNormal();
    const float CosHalfFov = FMath::Cos(FMath::DegreesToRadians(FOVAngle * 0.5f));
    const float MaxDist = Cfg.FOVSpawnRadius > 0.f ? Cfg.FOVSpawnRadius : 1e9f;

    const int32 N = (int32)W.Px.size();
    const int32 Threads = FMath::Clamp(FPlatformMisc::NumberOfCoresIncludingHyperthreads() - 1, 1, 8);
    const int32 Chunk = (N + Threads - 1) / Threads;
    FCriticalSection Lock;

    ParallelFor(Threads, [&](int32 tid)
    {
        const int32 Begin = tid * Chunk;
        const int32 End = FMath::Min(Begin + Chunk, N);
        if (Begin >= End) return;
        TArray<TArray<int32>> Local; Local.SetNum(Types);
        int32 LocalTotal = 0;
        for (int32 i = Begin; i < End; ++i)
        {
            if (!W.Alive[i]) continue;
            const FVector P(W.Px[i], W.Py[i], W.Pz[i]);
            const FVector To = (P - CamPos);
            const float Dist = To.Size();
            if (Cfg.DistanceCull && Dist > MaxDist) continue;
            if (Cfg.FrustumCull)
            {
                const FVector Dir = Dist > 1e-3f ? To / Dist : FVector::ZeroVector;
                if (FVector::DotProduct(Fwd, Dir) < CosHalfFov) continue;
            }
            const int32 T = (int32)W.Type[i];
            if (Local.IsValidIndex(T)) { Local[T].Add(i); LocalTotal++; }
        }
        if (LocalTotal > 0)
        {
            FScopeLock _( &Lock );
            for (int32 t = 0; t < Types; ++t)
            {
                if (Local[t].Num() > 0)
                {
                    VisibleByType[t].Append(Local[t]);
                }
            }
            VisibleTotal += LocalTotal;
        }
    });
}
