#include "Services/EnemyMovementService.h"

#include "Fragments/EnemyAgentFragment.h"
#include "Fragments/EnemyCombatFragment.h"
#include "Fragments/EnemyTargetFragment.h"

#include "Trace/Trace.h"
#include "Math/VectorRegister.h"

void UEnemyMovementService::Initialize(float InMaxSteeringForce, float InSeparationRadius, float InSeparationWeight)
{
    MaxSteeringForce = FMath::Max(0.0f, InMaxSteeringForce);
    SeparationRadius = FMath::Max(0.0f, InSeparationRadius);
    SeparationWeight = FMath::Max(0.0f, InSeparationWeight);
}

void UEnemyMovementService::UpdateAgent(FEnemyAgentFragment& AgentFragment,
    const FEnemyTargetFragment& TargetFragment,
    const FEnemyCombatFragment& CombatFragment,
    TConstArrayView<FEnemyAgentFragment> NeighbourAgents,
    TConstArrayView<FEnemyCombatFragment> CombatFragments,
    int32 AgentIndex,
    float DeltaTime) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(EnemyMovement_UpdateAgent);

    if (DeltaTime <= 0.0f)
    {
        return;
    }

    FVector DesiredDirection = TargetFragment.DesiredDirection;
    if (!TargetFragment.TacticalOffset.IsNearlyZero())
    {
        DesiredDirection += TargetFragment.TacticalOffset;
    }

    if (DesiredDirection.IsNearlyZero())
    {
        DesiredDirection = (TargetFragment.TargetLocation - AgentFragment.Position).GetSafeNormal();
    }

    FVector DesiredDirNorm = DesiredDirection.GetSafeNormal();
    if (DesiredDirNorm.IsNearlyZero())
    {
        DesiredDirNorm = AgentFragment.Velocity.IsNearlyZero() ? FVector::ForwardVector : AgentFragment.Velocity.GetSafeNormal();
    }

    float AppliedSpeed = FMath::Max(0.0f, AgentFragment.DesiredSpeed * FMath::Max(TargetFragment.DesiredSpeedScale, 0.1f));
    if (CombatFragment.bDashActive)
    {
        AppliedSpeed *= CombatFragment.DashSpeedMultiplier;
    }

    FVector Steering;
    {
        const VectorRegister DesiredVelocityReg = VectorMultiply(VectorLoadFloat3_W0(&DesiredDirNorm), VectorSetFloat1(AppliedSpeed));
        const VectorRegister VelocityReg = VectorLoadFloat3_W0(&AgentFragment.Velocity);
        const VectorRegister SteeringReg = VectorSubtract(DesiredVelocityReg, VelocityReg);
        VectorStoreFloat3(SteeringReg, &Steering);
    }
    Steering = Steering.GetClampedToMaxSize(MaxSteeringForce);

    const float SeparationRadiusSq = FMath::Square(SeparationRadius);
    const bool bComputeGroup = (CombatFragment.CohesionWeight > KINDA_SMALL_NUMBER || CombatFragment.AlignmentWeight > KINDA_SMALL_NUMBER);
    const float GroupRadius = bComputeGroup ? FMath::Max(CombatFragment.GroupRadius, SeparationRadius) : 0.0f;
    const float GroupRadiusSq = bComputeGroup ? FMath::Square(GroupRadius) : 0.0f;

    FVector SeparationForce = FVector::ZeroVector;
    FVector CohesionSum = FVector::ZeroVector;
    FVector AlignmentSum = FVector::ZeroVector;
    int32 CohesionCount = 0;

    const int32 NumAgents = NeighbourAgents.Num();
    for (int32 OtherIndex = 0; OtherIndex < NumAgents; ++OtherIndex)
    {
        if (OtherIndex == AgentIndex)
        {
            continue;
        }

        const FVector Delta = AgentFragment.Position - NeighbourAgents[OtherIndex].Position;
        const float DistSq = Delta.SizeSquared();

        if (SeparationRadiusSq > KINDA_SMALL_NUMBER && DistSq < SeparationRadiusSq && DistSq > KINDA_SMALL_NUMBER)
        {
            const float Dist = FMath::Sqrt(DistSq);
            const float Penetration = SeparationRadius - Dist;
            SeparationForce += Delta.GetSafeNormal() * Penetration;
        }

        if (bComputeGroup && DistSq <= GroupRadiusSq)
        {
            const float OtherRadius = CombatFragments.IsValidIndex(OtherIndex)
                ? FMath::Max(CombatFragments[OtherIndex].GroupRadius, 1.0f)
                : GroupRadius;
            const float EffectiveRadius = FMath::Min(GroupRadius, OtherRadius);

            if (DistSq <= FMath::Square(EffectiveRadius))
            {
                CohesionSum += NeighbourAgents[OtherIndex].Position;
                AlignmentSum += NeighbourAgents[OtherIndex].Velocity;
                ++CohesionCount;
            }
        }
    }

    if (!SeparationForce.IsNearlyZero())
    {
        Steering += SeparationForce.GetClampedToMaxSize(SeparationWeight);
    }

    if (bComputeGroup && CohesionCount > 0)
    {
        const FVector AveragePosition = CohesionSum / static_cast<float>(CohesionCount);
        Steering += (AveragePosition - AgentFragment.Position).GetClampedToMaxSize(MaxSteeringForce) * CombatFragment.CohesionWeight;

        FVector AverageVelocity = AlignmentSum / static_cast<float>(CohesionCount);
        if (!AverageVelocity.IsNearlyZero() && !AgentFragment.Velocity.IsNearlyZero())
        {
            const FVector Alignment = (AverageVelocity.GetSafeNormal() - AgentFragment.Velocity.GetSafeNormal()).GetClampedToMaxSize(MaxSteeringForce);
            Steering += Alignment * CombatFragment.AlignmentWeight;
        }
    }

    AgentFragment.Velocity += Steering * DeltaTime;
    const float MaxSpeed = FMath::Max(AppliedSpeed, 10.0f);
    AgentFragment.Velocity = AgentFragment.Velocity.GetClampedToMaxSize(MaxSpeed);
    AgentFragment.Position += AgentFragment.Velocity * DeltaTime;
    AgentFragment.Position.Z = 0.0f;

    AgentFragment.CachedTransform.SetLocation(AgentFragment.Position);
    if (!AgentFragment.Velocity.IsNearlyZero())
    {
        AgentFragment.CachedTransform.SetRotation(FQuat::FindBetweenNormals(FVector::ForwardVector, AgentFragment.Velocity.GetSafeNormal()));
    }
}
