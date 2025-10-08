#include "Enemy/Types/FallenWarlordBoss.h"
#include "Kismet/GameplayStatics.h"
// TODO: FallenSwordWeapon was removed with Swarm system - implement boss weapon if needed
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Logging/VazioLogFacade.h"

AFallenWarlordBoss::AFallenWarlordBoss()
{
    BossDisplayName = FText::FromString(TEXT("Fallen Warlord"));

    SweepDamage = 45.f;
    FlameWaveDamage = 35.f;
    FlameWaveRadius = 700.f;
    EarthShatterRange = 900.f;

    MovementRadius = 450.f;
    MovementInterpSpeed = 0.8f;
    MovementHeightOffset = 0.f;
    bFacePlayer = true;

    DefaultAttackInterval = 6.f;
    bLoopPhasePatterns = true;

    FBossAttackPattern BladeSweep;
    BladeSweep.PatternName = TEXT("BladeSweep");
    BladeSweep.TelegraphTime = 2.0f;
    BladeSweep.Cooldown = 7.f;

    FBossAttackPattern FlameWave;
    FlameWave.PatternName = TEXT("FlameWave");
    FlameWave.TelegraphTime = 1.5f;
    FlameWave.Cooldown = 8.f;

    FBossAttackPattern EarthShatter;
    EarthShatter.PatternName = TEXT("EarthShatter");
    EarthShatter.TelegraphTime = 2.5f;
    EarthShatter.Cooldown = 10.f;
    EarthShatter.bSummonsMinions = true;
    EarthShatter.MinionType = TEXT("HeavyEnemy");
    EarthShatter.MinionCount = 2;
    EarthShatter.MinionSpawnRadius = 400.f;

    FBossPhaseDefinition PhaseOne;
    PhaseOne.HealthThreshold = 1.0f;
    PhaseOne.AttackPatterns = { BladeSweep, FlameWave };
    PhaseOne.MovementSpeedMultiplier = 0.85f;

    FBossPhaseDefinition PhaseTwo;
    PhaseTwo.HealthThreshold = 0.55f;
    PhaseTwo.AttackPatterns = { BladeSweep, FlameWave, EarthShatter };
    PhaseTwo.MovementSpeedMultiplier = 1.1f;
    PhaseTwo.bEnableSummoningLoop = true;
    PhaseTwo.SummonInterval = 20.f;
    PhaseTwo.SummonType = TEXT("DashEnemy");
    PhaseTwo.SummonCount = 4;
    PhaseTwo.SummonRadius = 600.f;

    Phases = { PhaseOne, PhaseTwo };
}

void AFallenWarlordBoss::BeginPlay()
{
    Super::BeginPlay();

    // TODO: Weapon system - AFallenSwordWeapon class needs to be created first
    /*
    if (UWorld* World = GetWorld())
    {
        FActorSpawnParameters Params;
        Params.Owner = this;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        SwordWeapon = World->SpawnActor<AFallenSwordWeapon>(AFallenSwordWeapon::StaticClass(), GetActorTransform(), Params);
        if (SwordWeapon)
        {
            SwordWeapon->InitializeForBoss(this);
        }
        else
        {
            LOG_ENEMIES(Warn, TEXT("Failed to spawn FallenSwordWeapon for %s"), *GetName());
        }
    }
    */
}

void AFallenWarlordBoss::HandlePhaseStarted(const FBossPhaseDefinition& Phase)
{
    Super::HandlePhaseStarted(Phase);

    MovementRadius = Phase.HealthThreshold > 0.6f ? 450.f : 350.f;

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = CurrentArchetype.BaseSpeed * Phase.MovementSpeedMultiplier;
    }
}

void AFallenWarlordBoss::PerformAttackPattern(const FBossAttackPattern& Pattern)
{
    if (Pattern.PatternName == TEXT("BladeSweep"))
    {
        PerformGreatswordSweep();
    }
    else if (Pattern.PatternName == TEXT("FlameWave"))
    {
        PerformFlameWave();
    }
    else if (Pattern.PatternName == TEXT("EarthShatter"))
    {
        PerformEarthShatter();
    }
    else
    {
        Super::PerformAttackPattern(Pattern);
    }
}

void AFallenWarlordBoss::PerformMovementPattern(float DeltaTime)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        Super::PerformMovementPattern(DeltaTime);
        return;
    }

    const FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector DesiredLocation = PlayerLocation - PlayerPawn->GetActorForwardVector() * 350.f;
    DesiredLocation.Z = PlayerLocation.Z;

    const FVector NewLocation = FMath::VInterpTo(GetActorLocation(), DesiredLocation, DeltaTime, 0.9f);
    SetActorLocation(NewLocation);

    if (bFacePlayer)
    {
        const FVector Direction = (PlayerLocation - NewLocation).GetSafeNormal();
        if (!Direction.IsNearlyZero())
        {
            SetActorRotation(Direction.Rotation());
        }
    }
}

void AFallenWarlordBoss::PerformGreatswordSweep()
{
    // TODO: Weapon system - AFallenSwordWeapon class needs to be created first
    // For now, use fallback radial damage
    UGameplayStatics::ApplyRadialDamage(GetWorld(), SweepDamage, GetActorLocation(), 300.f, nullptr, TArray<AActor*>(), this);
}

void AFallenWarlordBoss::PerformFlameWave()
{
    // TODO: Weapon system - AFallenSwordWeapon class needs to be created first
    // For now, use fallback radial damage
    UGameplayStatics::ApplyRadialDamage(GetWorld(), FlameWaveDamage, GetActorLocation(), FlameWaveRadius, nullptr, TArray<AActor*>(), this);
}

void AFallenWarlordBoss::PerformEarthShatter()
{
    // TODO: Weapon system - AFallenSwordWeapon class needs to be created first
    // For now, use fallback radial damage
    FVector Origin = GetActorLocation();
    FVector Forward = GetActorForwardVector();
    FVector ImpactLocation = Origin + Forward * 200.f;
    UGameplayStatics::ApplyRadialDamage(GetWorld(), SweepDamage * 1.2f, ImpactLocation, EarthShatterRange, nullptr, TArray<AActor*>(), this);
}