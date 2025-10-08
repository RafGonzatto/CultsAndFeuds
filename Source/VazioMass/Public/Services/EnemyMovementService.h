#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EnemyMovementService.generated.h"

struct FEnemyAgentFragment;
struct FEnemyTargetFragment;
struct FEnemyCombatFragment;

/**
 * Domain service encapsulating shared movement calculations for enemy mass entities.
 */
UCLASS()
class VAZIOMASS_API UEnemyMovementService : public UObject
{
    GENERATED_BODY()

public:
    /** Configures runtime parameters. */
    void Initialize(float InMaxSteeringForce, float InSeparationRadius, float InSeparationWeight);

    /** Applies steering, velocity clamping, local avoidance, and cached transform updates to the provided agent fragment. */
    void UpdateAgent(FEnemyAgentFragment& AgentFragment,
        const FEnemyTargetFragment& TargetFragment,
        const FEnemyCombatFragment& CombatFragment,
        TConstArrayView<FEnemyAgentFragment> NeighbourAgents,
        TConstArrayView<FEnemyCombatFragment> CombatFragments,
        int32 AgentIndex,
        float DeltaTime) const;

private:
    float MaxSteeringForce = 1200.0f;
    float SeparationRadius = 150.0f;
    float SeparationWeight = 400.0f;
};
