#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "EnemyCombatFragment.generated.h"

UENUM(BlueprintType)
enum class EEnemyAbilityFlags : uint8
{
    None    = 0,
    Dash    = 1 << 0,
    Ranged  = 1 << 1,
    Aura    = 1 << 2
};
ENUM_CLASS_FLAGS(EEnemyAbilityFlags)

/**
 * Combat and advanced behavior data for mass-based enemies.
 */
USTRUCT()
struct VAZIOMASS_API FEnemyCombatFragment : public FMassFragment
{
    GENERATED_BODY()

    /** Effective distance at which the enemy prefers to execute its primary attack. */
    UPROPERTY(EditAnywhere, Category = "Enemy|Combat")
    float AttackRange = 180.0f;

    /** Cooldown between primary attacks. */
    UPROPERTY(EditAnywhere, Category = "Enemy|Combat", meta = (ClampMin = "0.05"))
    float AttackCooldown = 1.0f;

    /** Remaining time until another primary attack can be triggered. */
    UPROPERTY(Transient)
    float AttackTimer = 0.0f;

    /** Target spacing preferred for ranged archetypes. */
    UPROPERTY(EditAnywhere, Category = "Enemy|Combat")
    float PreferredRange = 520.0f;

    /** Boids-style cohesion to keep squads together. */
    UPROPERTY(EditAnywhere, Category = "Enemy|Group")
    float CohesionWeight = 0.45f;

    /** Boids-style alignment to keep squads moving in the same direction. */
    UPROPERTY(EditAnywhere, Category = "Enemy|Group")
    float AlignmentWeight = 0.35f;

    /** Maximum radius considered when computing group behaviors. */
    UPROPERTY(EditAnywhere, Category = "Enemy|Group")
    float GroupRadius = 600.0f;

    /** Cooldown between dash bursts (if dash ability is available). */
    UPROPERTY(EditAnywhere, Category = "Enemy|Abilities")
    float DashCooldown = 4.0f;

    /** Duration of the active dash burst. */
    UPROPERTY(EditAnywhere, Category = "Enemy|Abilities")
    float DashDuration = 0.6f;

    /** Speed multiplier while the dash is active. */
    UPROPERTY(EditAnywhere, Category = "Enemy|Abilities")
    float DashSpeedMultiplier = 2.4f;

    /** Remaining cooldown time for dash ability. */
    UPROPERTY(Transient)
    float DashTimer = 0.0f;

    /** Remaining active dash time when currently dashing. */
    UPROPERTY(Transient)
    float DashTimeRemaining = 0.0f;

    /** Bit-mask describing available special abilities. */
    UPROPERTY(EditAnywhere, Category = "Enemy|Abilities")
    EEnemyAbilityFlags AbilityFlags = EEnemyAbilityFlags::None;

    /** Pending attack flag consumed by the combat processor. */
    UPROPERTY(Transient)
    bool bRequestAttack = false;

    /** True while a dash burst is active. */
    UPROPERTY(Transient)
    bool bDashActive = false;

    FORCEINLINE bool HasAbility(EEnemyAbilityFlags Flag) const
    {
        return EnumHasAnyFlags(AbilityFlags, Flag);
    }
};
