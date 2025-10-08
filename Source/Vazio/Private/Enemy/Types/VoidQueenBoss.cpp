#include "Enemy/Types/VoidQueenBoss.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Logging/VazioLogFacade.h"

AVoidQueenBoss::AVoidQueenBoss()
{
    BossDisplayName = FText::FromString(TEXT("Void Queen"));

    SlamRadius = 450.f;
    SlamDamage = 30.f;
    DashDistance = 900.f;
    DashSpeedMultiplier = 3.5f;

    MovementRadius = 900.f;
    MovementInterpSpeed = 1.5f;
    MovementHeightOffset = 120.f;

    DefaultAttackInterval = 5.f;
    bLoopPhasePatterns = true;

    FBossAttackPattern TentacleSlam;
    TentacleSlam.PatternName = TEXT("TentacleSlam");
    TentacleSlam.TelegraphTime = 1.8f;
    TentacleSlam.Cooldown = 6.f;

    FBossAttackPattern VoidDash;
    VoidDash.PatternName = TEXT("VoidDash");
    VoidDash.TelegraphTime = 1.2f;
    VoidDash.Cooldown = 7.f;

    FBossAttackPattern BroodCall;
    BroodCall.PatternName = TEXT("BroodCall");
    BroodCall.TelegraphTime = 1.5f;
    BroodCall.Cooldown = 8.f;
    BroodCall.bSummonsMinions = true;
    BroodCall.MinionType = TEXT("DashEnemy");
    BroodCall.MinionCount = 4;
    BroodCall.MinionSpawnRadius = 650.f;

    FBossAttackPattern VoidNova;
    VoidNova.PatternName = TEXT("VoidNova");
    VoidNova.TelegraphTime = 2.0f;
    VoidNova.Cooldown = 9.f;

    FBossPhaseDefinition PhaseOne;
    PhaseOne.HealthThreshold = 1.0f;
    PhaseOne.AttackPatterns = { TentacleSlam, VoidDash, BroodCall };
    PhaseOne.MovementSpeedMultiplier = 1.0f;

    FBossPhaseDefinition PhaseTwo;
    PhaseTwo.HealthThreshold = 0.5f;
    PhaseTwo.AttackPatterns = { TentacleSlam, VoidDash, VoidNova, BroodCall };
    PhaseTwo.MovementSpeedMultiplier = 1.25f;
    PhaseTwo.bEnableSummoningLoop = true;
    PhaseTwo.SummonInterval = 14.f;
    PhaseTwo.SummonType = TEXT("RangedEnemy");
    PhaseTwo.SummonCount = 6;
    PhaseTwo.SummonRadius = 900.f;

    Phases = { PhaseOne, PhaseTwo };
}

void AVoidQueenBoss::HandlePhaseStarted(const FBossPhaseDefinition& Phase)
{
    Super::HandlePhaseStarted(Phase);

    MovementRadius = Phase.HealthThreshold > 0.6f ? 900.f : 650.f;

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = CurrentArchetype.BaseSpeed * Phase.MovementSpeedMultiplier;
    }
}

void AVoidQueenBoss::PerformAttackPattern(const FBossAttackPattern& Pattern)
{
    if (Pattern.PatternName == TEXT("TentacleSlam"))
    {
        PerformTentacleSlam();
    }
    else if (Pattern.PatternName == TEXT("VoidDash"))
    {
        PerformVoidDash();
    }
    else if (Pattern.PatternName == TEXT("BroodCall"))
    {
        PerformBroodSummon();
    }
    else if (Pattern.PatternName == TEXT("VoidNova"))
    {
        UGameplayStatics::ApplyRadialDamage(GetWorld(), SlamDamage * 1.5f, GetActorLocation(), SlamRadius * 1.25f, nullptr, TArray<AActor*>(), this);
        SpawnSummonedMinions(TEXT("AuraEnemy"), 3, SlamRadius + 200.f, FEnemyInstanceModifiers());
    }
    else
    {
        Super::PerformAttackPattern(Pattern);
    }
}

void AVoidQueenBoss::PerformMovementPattern(float DeltaTime)
{
    MovementRadius = FMath::FInterpTo(MovementRadius, (GetCurrentPhaseIndex() == 0) ? 900.f : 620.f, DeltaTime, 0.5f);
    Super::PerformMovementPattern(DeltaTime);
}

void AVoidQueenBoss::PerformTentacleSlam()
{
    UGameplayStatics::ApplyRadialDamage(GetWorld(), SlamDamage, GetActorLocation(), SlamRadius, nullptr, TArray<AActor*>(), this);
    LOG_ENEMIES(Info, TEXT("%s performed Tentacle Slam"), *GetName());
}

void AVoidQueenBoss::PerformVoidDash()
{
    DashTowardsPlayer(DashDistance, MovementHeightOffset);
    LOG_ENEMIES(Info, TEXT("%s performed Void Dash"), *GetName());
}

void AVoidQueenBoss::PerformBroodSummon()
{
    LOG_ENEMIES(Info, TEXT("%s calls forth brood minions"), *GetName());
}

void AVoidQueenBoss::DashTowardsPlayer(float Distance, float HeightOffset)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    const FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
    FVector Direction = ToPlayer.GetSafeNormal();

    if (Direction.IsNearlyZero())
    {
        return;
    }

    FVector TargetLocation = PlayerPawn->GetActorLocation() - Direction * 150.f;
    TargetLocation.Z += HeightOffset;

    FVector NewLocation = FMath::VInterpTo(GetActorLocation(), TargetLocation, GetWorld()->GetDeltaSeconds(), DashSpeedMultiplier * 5.f);
    SetActorLocation(NewLocation);

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->Velocity = Direction * DashDistance;
    }
}
