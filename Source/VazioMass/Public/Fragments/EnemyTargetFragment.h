#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "EnemyTargetFragment.generated.h"

/**
 * Fragment describing target-seeking data for a mass enemy.
 */
USTRUCT()
struct VAZIOMASS_API FEnemyTargetFragment : public FMassFragment
{
    GENERATED_BODY()

    /** Target actor location enemies should seek toward. */
    UPROPERTY(EditAnywhere, Category = "Enemy|Targeting")
    FVector TargetLocation = FVector::ZeroVector;

    /** Desired movement direction computed by navigation processors. */
    UPROPERTY(Transient)
    FVector DesiredDirection = FVector::ForwardVector;

    /** Optional multiplier applied on top of the agent's desired speed. */
    UPROPERTY(Transient)
    float DesiredSpeedScale = 1.0f;

    /** Per-tick tactical offset such as strafing vectors. */
    UPROPERTY(Transient)
    FVector TacticalOffset = FVector::ZeroVector;

    /** Whether the agent decided to execute a dash burst this frame. */
    UPROPERTY(Transient)
    bool bWantsDash = false;

    /** Whether the agent wants to trigger its primary attack this frame. */
    UPROPERTY(Transient)
    bool bWantsAttack = false;
};
