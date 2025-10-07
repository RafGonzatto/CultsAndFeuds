#include "Enemy/Types/HybridDemonBoss.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

AHybridDemonBoss::AHybridDemonBoss()
{
    BossDisplayName = FText::FromString(TEXT("Hybrid Demon"));

    InfernoDamage = 32.f;
    InfernoRadius = 800.f;
    ShadowDashDistance = 1000.f;
    CataclysmDamage = 55.f;

    MovementRadius = 850.f;
    MovementInterpSpeed = 2.0f;
    MovementHeightOffset = 160.f;
    bFacePlayer = true;

    DefaultAttackInterval = 5.0f;
    bLoopPhasePatterns = true;

    FBossAttackPattern InfernoRain;
    InfernoRain.PatternName = TEXT("InfernoRain");
    InfernoRain.TelegraphTime = 2.0f;
    InfernoRain.Cooldown = 8.0f;
    InfernoRain.bSummonsMinions = true;
    InfernoRain.MinionType = TEXT("RangedEnemy");
    InfernoRain.MinionCount = 5;
    InfernoRain.MinionSpawnRadius = 900.f;

    FBossAttackPattern ShadowDash;
    ShadowDash.PatternName = TEXT("ShadowDash");
    ShadowDash.TelegraphTime = 1.0f;
    ShadowDash.Cooldown = 6.0f;

    FBossAttackPattern OblivionVolley;
    OblivionVolley.PatternName = TEXT("OblivionProjectiles");
    OblivionVolley.TelegraphTime = 1.8f;
    OblivionVolley.Cooldown = 9.0f;

    FBossAttackPattern Cataclysm;
    Cataclysm.PatternName = TEXT("Cataclysm");
    Cataclysm.TelegraphTime = 2.5f;
    Cataclysm.Cooldown = 11.0f;
    Cataclysm.bSummonsMinions = true;
    Cataclysm.MinionType = TEXT("AuraEnemy");
    Cataclysm.MinionCount = 4;
    Cataclysm.MinionSpawnRadius = 700.f;

    FBossPhaseDefinition PhaseOne;
    PhaseOne.HealthThreshold = 1.0f;
    PhaseOne.AttackPatterns = { InfernoRain, ShadowDash };
    PhaseOne.MovementSpeedMultiplier = 1.0f;

    FBossPhaseDefinition PhaseTwo;
    PhaseTwo.HealthThreshold = 0.65f;
    PhaseTwo.AttackPatterns = { InfernoRain, ShadowDash, OblivionVolley };
    PhaseTwo.MovementSpeedMultiplier = 1.2f;
    PhaseTwo.bEnableSummoningLoop = true;
    PhaseTwo.SummonInterval = 16.f;
    PhaseTwo.SummonType = TEXT("DashEnemy");
    PhaseTwo.SummonCount = 6;
    PhaseTwo.SummonRadius = 800.f;

    FBossPhaseDefinition PhaseThree;
    PhaseThree.HealthThreshold = 0.35f;
    PhaseThree.AttackPatterns = { ShadowDash, OblivionVolley, Cataclysm, InfernoRain };
    PhaseThree.MovementSpeedMultiplier = 1.35f;
    PhaseThree.bEnableSummoningLoop = true;
    PhaseThree.SummonInterval = 12.f;
    PhaseThree.SummonType = TEXT("HeavyEnemy");
    PhaseThree.SummonCount = 4;
    PhaseThree.SummonRadius = 850.f;

    Phases = { PhaseOne, PhaseTwo, PhaseThree };
}

void AHybridDemonBoss::HandlePhaseStarted(const FBossPhaseDefinition& Phase)
{
    Super::HandlePhaseStarted(Phase);

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = CurrentArchetype.BaseSpeed * Phase.MovementSpeedMultiplier;
    }
}

void AHybridDemonBoss::PerformAttackPattern(const FBossAttackPattern& Pattern)
{
    if (Pattern.PatternName == TEXT("InfernoRain"))
    {
        PerformInfernoRain();
    }
    else if (Pattern.PatternName == TEXT("ShadowDash"))
    {
        PerformShadowDash();
    }
    else if (Pattern.PatternName == TEXT("OblivionProjectiles"))
    {
        PerformOblivionProjectiles();
    }
    else if (Pattern.PatternName == TEXT("Cataclysm"))
    {
        PerformCataclysm();
    }
    else
    {
        Super::PerformAttackPattern(Pattern);
    }
}

void AHybridDemonBoss::PerformMovementPattern(float DeltaTime)
{
    MovementRadius = FMath::FInterpTo(MovementRadius, (GetCurrentPhaseIndex() >= 2) ? 950.f : 850.f, DeltaTime, 0.8f);
    Super::PerformMovementPattern(DeltaTime);
}

void AHybridDemonBoss::PerformInfernoRain()
{
    UGameplayStatics::ApplyRadialDamage(GetWorld(), InfernoDamage, GetActorLocation(), InfernoRadius, nullptr, TArray<AActor*>(), this);
    UE_LOG(LogBoss, Log, TEXT("%s cast Inferno Rain"), *GetName());
}

void AHybridDemonBoss::PerformShadowDash()
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

    FVector TargetLocation = PlayerPawn->GetActorLocation() - Direction * 120.f;
    TargetLocation.Z = PlayerPawn->GetActorLocation().Z + MovementHeightOffset;

    SetActorLocation(TargetLocation);
    SetActorRotation((PlayerPawn->GetActorLocation() - TargetLocation).Rotation());

    UE_LOG(LogBoss, Log, TEXT("%s executed Shadow Dash"), *GetName());
}

void AHybridDemonBoss::PerformOblivionProjectiles()
{
    if (!GetWorld())
    {
        return;
    }

    const FVector Origin = GetActorLocation();
    for (int32 Index = 0; Index < 6; ++Index)
    {
        const float Angle = (2.f * PI * Index) / 6;
        FVector Offset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * 600.f;
        FVector ImpactLocation = Origin + Offset;
        UGameplayStatics::ApplyRadialDamage(GetWorld(), InfernoDamage * 0.75f, ImpactLocation, 280.f, nullptr, TArray<AActor*>(), this);
    }

    UE_LOG(LogBoss, Log, TEXT("%s released Oblivion projectiles"), *GetName());
}

void AHybridDemonBoss::PerformCataclysm()
{
    FVector Origin = GetActorLocation();
    UGameplayStatics::ApplyRadialDamage(GetWorld(), CataclysmDamage, Origin, InfernoRadius * 1.2f, nullptr, TArray<AActor*>(), this);
    SpawnSummonedMinions(TEXT("GoldEnemy"), 2, InfernoRadius, FEnemyInstanceModifiers());
    UE_LOG(LogBoss, Log, TEXT("%s unleashed Cataclysm"), *GetName());
}

