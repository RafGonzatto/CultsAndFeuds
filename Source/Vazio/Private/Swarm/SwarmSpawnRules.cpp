#include "Swarm/SwarmSpawnRules.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Swarm/SwarmConfig.h"
#include "Logging/LogMacros.h"

namespace SwarmSpawn
{
    static TAutoConsoleVariable<int32> CVarSwarmDebugSpawn(TEXT("swarm.DebugSpawn"), 1, TEXT("Log spawn attempts/results per frame (0/1)"), ECVF_Cheat);
    static FORCEINLINE FVector RandomInUnitDisk2D(FRandomStream& Rng)
    {
        // Concentric map would be nicer; disk rejection is fine here
        for (int i = 0; i < 8; ++i)
        {
            const float x = Rng.FRandRange(-1.f, 1.f);
            const float y = Rng.FRandRange(-1.f, 1.f);
            if (x * x + y * y <= 1.f) return FVector(x, y, 0.f);
        }
        return FVector::ZeroVector;
    }

    bool FindSpawnPointInView(const UWorld& World, const FVector& CamPos, const FRotator& CamRot,
                              float Near, float Far, float Radius, FVector& OutPoint)
    {
        if (!World.IsGameWorld())
        {
            return false;
        }

        FRandomStream Rng(FPlatformTime::Cycles());
        const FVector Fwd = CamRot.Vector();
        const FVector Right = FRotationMatrix(CamRot).GetUnitAxis(EAxis::Y);
        const FVector Up = FRotationMatrix(CamRot).GetUnitAxis(EAxis::Z);

        const float Dist = Rng.FRandRange(Near, Far);
        const FVector Disk = RandomInUnitDisk2D(Rng) * Radius;
        const FVector Center = CamPos + Fwd * Dist;
        OutPoint = Center + Right * Disk.X + Up * Disk.Y;
        return true;
    }

    bool ProjectToNavmeshOrGround(const UWorld& World, const FVector& InPoint, bool bUseNavmesh,
                                  FVector& OutPoint)
    {
        OutPoint = InPoint;
        if (bUseNavmesh)
        {
            if (const UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(&World))
            {
                FNavLocation Loc;
                if (Nav->ProjectPointToNavigation(InPoint, Loc, FVector(300.f, 300.f, 500.f)))
                {
                    OutPoint = Loc.Location;
                    if (CVarSwarmDebugSpawn.GetValueOnGameThread() != 0)
                    {
                        UE_LOG(LogSwarm, VeryVerbose, TEXT("[SpawnRules] NavMesh OK -> %s"), *OutPoint.ToCompactString());
                    }
                    return true;
                }
                else if (CVarSwarmDebugSpawn.GetValueOnGameThread() != 0)
                {
                    UE_LOG(LogSwarm, VeryVerbose, TEXT("[SpawnRules] NavMesh FAIL for %s"), *InPoint.ToCompactString());
                }
            }
        }

        // Fallback: line trace downwards to find ground
        FHitResult Hit;
        FVector Start = InPoint + FVector(0, 0, 2000.f);
        FVector End = InPoint - FVector(0, 0, 5000.f);
        FCollisionQueryParams Q(SCENE_QUERY_STAT(SwarmGroundTrace), false);
        if (World.LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Q))
        {
            OutPoint = Hit.ImpactPoint + FVector(0, 0, 5.f);
            if (CVarSwarmDebugSpawn.GetValueOnGameThread() != 0)
            {
                UE_LOG(LogSwarm, VeryVerbose, TEXT("[SpawnRules] Trace OK -> %s"), *OutPoint.ToCompactString());
            }
            return true;
        }
        else if (CVarSwarmDebugSpawn.GetValueOnGameThread() != 0)
        {
            UE_LOG(LogSwarm, VeryVerbose, TEXT("[SpawnRules] Trace FAIL for %s"), *InPoint.ToCompactString());
        }
        return false;
    }
}
