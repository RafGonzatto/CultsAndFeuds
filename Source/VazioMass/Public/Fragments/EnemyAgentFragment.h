#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "EnemyAgentFragment.generated.h"

/**
 * Core fragment describing a mass-based enemy agent's kinematics and runtime state.
 */
USTRUCT()
struct alignas(16) VAZIOMASS_API FEnemyAgentFragment : public FMassFragment
{
    GENERATED_BODY()

    /** World-space position used by movement processors (kept separate from transform fragment for SIMD-friendly updates). */
    UPROPERTY(EditAnywhere, Category = "Enemy|Movement")
    FVector Position = FVector::ZeroVector;

    /** World-space velocity accumulated by the movement processor. */
    UPROPERTY(EditAnywhere, Category = "Enemy|Movement")
    FVector Velocity = FVector::ZeroVector;

    /** Desired cruising speed for the agent. */
    UPROPERTY(EditAnywhere, Category = "Enemy|Movement", meta = (ClampMin = "0.0"))
    float DesiredSpeed = 300.0f;

    /** Identifier linking the agent to an archetype template and shared config data. */
    UPROPERTY(EditAnywhere, Category = "Enemy|Config")
    int32 ArchetypeID = 0;

    /** Current health value for the agent. */
    UPROPERTY(EditAnywhere, Category = "Enemy|Combat", meta = (ClampMin = "0.0"))
    float Health = 100.0f;

    /** Cached transform for fast visualization and instancing. */
    FTransform CachedTransform;

    /** Server-assigned unique identifier for replication. */
    UPROPERTY(VisibleAnywhere, Category = "Enemy|Network")
    int32 NetID = INDEX_NONE;
};
