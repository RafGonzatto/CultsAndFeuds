#include "Enemy/Types/BurrowerBoss.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Logging/VazioLogFacade.h"

ABurrowerBoss::ABurrowerBoss()
{
    BossDisplayName = FText::FromString(TEXT("Burrower Prime"));

    bIsBurrowed = false;
    BurrowDuration = 2.5f;
    SpikeDamage = 28.f;
    SpikeRadius = 500.f;

    MovementRadius = 0.f;
    MovementInterpSpeed = 1.0f;
    MovementHeightOffset = -50.f;
    bFacePlayer = true;

    DefaultAttackInterval = 4.5f;
    bLoopPhasePatterns = true;

    FBossAttackPattern Burrow;
    Burrow.PatternName = TEXT("Burrow");
    Burrow.TelegraphTime = 1.0f;
    Burrow.Cooldown = 6.f;

    FBossAttackPattern Ambush;
    Ambush.PatternName = TEXT("Ambush");
    Ambush.TelegraphTime = 1.0f;
    Ambush.Cooldown = 6.f;

    FBossAttackPattern SpikeBurst;
    SpikeBurst.PatternName = TEXT("SpikeBurst");
    SpikeBurst.TelegraphTime = 1.2f;
    SpikeBurst.Cooldown = 7.f;

    FBossAttackPattern SummonLarvae;
    SummonLarvae.PatternName = TEXT("SummonLarvae");
    SummonLarvae.TelegraphTime = 1.5f;
    SummonLarvae.Cooldown = 9.f;
    SummonLarvae.bSummonsMinions = true;
    SummonLarvae.MinionType = TEXT("NormalEnemy");
    SummonLarvae.MinionCount = 5;
    SummonLarvae.MinionSpawnRadius = 450.f;

    FBossPhaseDefinition PhaseOne;
    PhaseOne.HealthThreshold = 1.0f;
    PhaseOne.AttackPatterns = { Burrow, Ambush, SpikeBurst };
    PhaseOne.MovementSpeedMultiplier = 0.9f;

    FBossPhaseDefinition PhaseTwo;
    PhaseTwo.HealthThreshold = 0.4f;
    PhaseTwo.AttackPatterns = { Burrow, Ambush, SpikeBurst, SummonLarvae };
    PhaseTwo.MovementSpeedMultiplier = 1.15f;
    PhaseTwo.bEnableSummoningLoop = true;
    PhaseTwo.SummonInterval = 18.f;
    PhaseTwo.SummonType = TEXT("SplitterSlime");
    PhaseTwo.SummonCount = 6;
    PhaseTwo.SummonRadius = 550.f;

    Phases = { PhaseOne, PhaseTwo };
}

void ABurrowerBoss::HandlePhaseStarted(const FBossPhaseDefinition& Phase)
{
    Super::HandlePhaseStarted(Phase);

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = CurrentArchetype.BaseSpeed * Phase.MovementSpeedMultiplier;
    }
}

void ABurrowerBoss::PerformAttackPattern(const FBossAttackPattern& Pattern)
{
    if (Pattern.PatternName == TEXT("Burrow"))
    {
        StartBurrow();
    }
    else if (Pattern.PatternName == TEXT("Ambush"))
    {
        PerformAmbushStrike();
    }
    else if (Pattern.PatternName == TEXT("SpikeBurst"))
    {
        PerformSpikeBurst();
    }
    else if (Pattern.PatternName == TEXT("SummonLarvae"))
    {
    LOG_ENEMIES(Info, TEXT("%s summons larvae"), *GetName());
    }
    else
    {
        Super::PerformAttackPattern(Pattern);
    }
}

void ABurrowerBoss::PerformMovementPattern(float DeltaTime)
{
    if (bIsBurrowed)
    {
        return;
    }

    Super::PerformMovementPattern(DeltaTime);
}

float ABurrowerBoss::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsBurrowed)
    {
        LOG_ENEMIES(Trace, TEXT("%s ignored damage while burrowed"), *GetName());
        return 0.f;
    }

    return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void ABurrowerBoss::StartBurrow()
{
    if (bIsBurrowed)
    {
        return;
    }

    bIsBurrowed = true;

    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(BurrowTimerHandle, this, &ABurrowerBoss::FinishBurrow, BurrowDuration, false);
    }

    LOG_ENEMIES(Info, TEXT("%s burrowed underground"), *GetName());
}

void ABurrowerBoss::FinishBurrow()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(BurrowTimerHandle);
    }

    if (!bIsBurrowed)
    {
        return;
    }

    bIsBurrowed = false;

    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);

    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        FVector BackDirection = -PlayerPawn->GetActorForwardVector();
        FVector NewLocation = PlayerPawn->GetActorLocation() + BackDirection * 200.f;
        NewLocation.Z = PlayerPawn->GetActorLocation().Z - 20.f;
        SetActorLocation(NewLocation);
        SetActorRotation((PlayerPawn->GetActorLocation() - NewLocation).Rotation());
    }

    LOG_ENEMIES(Info, TEXT("%s resurfaced"), *GetName());
}

void ABurrowerBoss::PerformAmbushStrike()
{
    FinishBurrow();
    UGameplayStatics::ApplyRadialDamage(GetWorld(), SpikeDamage * 1.2f, GetActorLocation(), 250.f, nullptr, TArray<AActor*>(), this);
    LOG_ENEMIES(Info, TEXT("%s performed an ambush strike"), *GetName());
}

void ABurrowerBoss::PerformSpikeBurst()
{
    UGameplayStatics::ApplyRadialDamage(GetWorld(), SpikeDamage, GetActorLocation(), SpikeRadius, nullptr, TArray<AActor*>(), this);
    LOG_ENEMIES(Info, TEXT("%s released a spike burst"), *GetName());
}
