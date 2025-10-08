#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EnemyAIService.generated.h"

struct FEnemyAgentFragment;
struct FEnemyTargetFragment;
struct FEnemyCombatFragment;

/**
 * Encapsulates reusable combat/movement decision logic for mass-based enemies.
 */
UCLASS()
class VAZIOMASS_API UEnemyAIService : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(float InChaseSpeed, float InAttackSpeed, float InApproachDistance);

    /** Computes desired direction and speed modifiers for a given agent. */
    void EvaluateAgent(const FVector& PrimaryTargetLocation,
        FEnemyAgentFragment& AgentFragment,
        FEnemyTargetFragment& TargetFragment,
        FEnemyCombatFragment& CombatFragment,
        float DeltaTime) const;

private:
    float ChaseSpeed = 450.0f;
    float AttackSpeed = 200.0f;
    float ApproachDistance = 350.0f;
};
