#pragma once
#include "CoreMinimal.h"

namespace SwarmSpawn
{
    // Sample a visible point roughly inside the camera frustum using a planar ring [Near, Far].
    // Returns true if a candidate was produced.
    bool FindSpawnPointInView(const UWorld& World, const FVector& CamPos, const FRotator& CamRot,
                              float Near, float Far, float Radius, FVector& OutPoint);

    // Project candidate to NavMesh (reachable) or ground via downward line trace. Returns true if valid.
    bool ProjectToNavmeshOrGround(const UWorld& World, const FVector& InPoint, bool bUseNavmesh,
                                  FVector& OutPoint);
}

