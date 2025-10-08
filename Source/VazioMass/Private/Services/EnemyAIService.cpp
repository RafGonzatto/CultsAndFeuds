#include "Services/EnemyAIService.h"

#include "Fragments/EnemyAgentFragment.h"
#include "Fragments/EnemyCombatFragment.h"
#include "Fragments/EnemyTargetFragment.h"

#include "Trace/Trace.h"

void UEnemyAIService::Initialize(float InChaseSpeed, float InAttackSpeed, float InApproachDistance)
{
    ChaseSpeed = FMath::Max(0.0f, InChaseSpeed);
    AttackSpeed = FMath::Max(0.0f, InAttackSpeed);
    ApproachDistance = FMath::Max(0.0f, InApproachDistance);
}

void UEnemyAIService::EvaluateAgent(const FVector& PrimaryTargetLocation,
    FEnemyAgentFragment& AgentFragment,
    FEnemyTargetFragment& TargetFragment,
    FEnemyCombatFragment& CombatFragment,
    float DeltaTime) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(EnemyAI_EvaluateAgent);

    CombatFragment.AttackTimer = FMath::Max(0.0f, CombatFragment.AttackTimer - DeltaTime);
    CombatFragment.DashTimer = FMath::Max(0.0f, CombatFragment.DashTimer - DeltaTime);
    if (CombatFragment.bDashActive)
    {
        CombatFragment.DashTimeRemaining = FMath::Max(0.0f, CombatFragment.DashTimeRemaining - DeltaTime);
        if (CombatFragment.DashTimeRemaining <= KINDA_SMALL_NUMBER)
        {
            CombatFragment.bDashActive = false;
        }
    }

    CombatFragment.bRequestAttack = false;
    TargetFragment.DesiredSpeedScale = 1.0f;
    TargetFragment.TacticalOffset = FVector::ZeroVector;
    TargetFragment.bWantsDash = CombatFragment.bDashActive;
    TargetFragment.bWantsAttack = false;

    if (PrimaryTargetLocation.IsNearlyZero())
    {
        TargetFragment.DesiredDirection = AgentFragment.Velocity.IsNearlyZero()
            ? FVector::ForwardVector
            : AgentFragment.Velocity.GetSafeNormal();
        return;
    }

    const FVector ToTarget = PrimaryTargetLocation - AgentFragment.Position;
    const float DistanceSq = ToTarget.SizeSquared();
    const bool bIsClose = DistanceSq <= FMath::Square(ApproachDistance);

    FVector DesiredDirection = ToTarget.GetSafeNormal();
    TargetFragment.TargetLocation = PrimaryTargetLocation;
    TargetFragment.DesiredDirection = DesiredDirection;

    const float DesiredSpeed = bIsClose ? AttackSpeed : ChaseSpeed;
    AgentFragment.DesiredSpeed = FMath::Max(DesiredSpeed, 150.0f);

    if (CombatFragment.HasAbility(EEnemyAbilityFlags::Dash) && !CombatFragment.bDashActive && CombatFragment.DashTimer <= 0.0f)
    {
        const float DashStartDistanceSq = FMath::Square(CombatFragment.AttackRange * 1.5f);
        if (DistanceSq >= DashStartDistanceSq)
        {
            CombatFragment.bDashActive = true;
            CombatFragment.DashTimeRemaining = CombatFragment.DashDuration;
            CombatFragment.DashTimer = CombatFragment.DashCooldown;
            TargetFragment.bWantsDash = true;
            TargetFragment.DesiredSpeedScale = FMath::Max(TargetFragment.DesiredSpeedScale, CombatFragment.DashSpeedMultiplier);
        }
    }

    if (CombatFragment.bDashActive)
    {
        TargetFragment.DesiredSpeedScale = FMath::Max(TargetFragment.DesiredSpeedScale, CombatFragment.DashSpeedMultiplier);
    }

    if (CombatFragment.HasAbility(EEnemyAbilityFlags::Ranged))
    {
        const float PreferredRangeSq = FMath::Square(CombatFragment.PreferredRange);
        if (DistanceSq < PreferredRangeSq * 0.5f)
        {
            TargetFragment.DesiredDirection = (-DesiredDirection).GetSafeNormal();
            TargetFragment.DesiredSpeedScale = FMath::Max(TargetFragment.DesiredSpeedScale, 1.15f);
        }
        else if (DistanceSq < PreferredRangeSq)
        {
            const FVector StrafeDir = FVector::CrossProduct(ToTarget, FVector::UpVector).GetSafeNormal();
            if (!StrafeDir.IsNearlyZero())
            {
                const float StrafeSign = ((AgentFragment.ArchetypeID & 1) ? 1.0f : -1.0f);
                TargetFragment.TacticalOffset = StrafeDir * StrafeSign;
            }
        }
        else
        {
            TargetFragment.DesiredSpeedScale = FMath::Max(TargetFragment.DesiredSpeedScale, 1.2f);
        }
    }

    if (CombatFragment.AttackTimer <= 0.0f && DistanceSq <= FMath::Square(CombatFragment.AttackRange))
    {
        CombatFragment.AttackTimer = CombatFragment.AttackCooldown;
        CombatFragment.bRequestAttack = true;
        TargetFragment.bWantsAttack = true;
    }

    TargetFragment.DesiredDirection = TargetFragment.DesiredDirection.GetSafeNormal();
}
