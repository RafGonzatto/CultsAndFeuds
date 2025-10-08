#include "Enemy/Types/BossEnemy.h"
#include "Enemy/EnemySpawnerSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Systems/EnemyMassSpawnerSubsystem.h"
#include "Systems/EnemyImplementationCVars.h"
#include "Logging/VazioLogFacade.h"

FBossAttackPattern::FBossAttackPattern()
{
    PatternName = NAME_None;
    TelegraphTime = 1.5f;
    Cooldown = 6.f;
    ExecutionTime = 1.f;
    bSummonsMinions = false;
    MinionType = NAME_None;
    MinionCount = 0;
    MinionSpawnRadius = 600.f;
}

FBossPhaseDefinition::FBossPhaseDefinition()
{
    HealthThreshold = 1.f;
    MovementSpeedMultiplier = 1.f;
    bEnableSummoningLoop = false;
    SummonInterval = 10.f;
    SummonType = NAME_None;
    SummonCount = 0;
    SummonRadius = 600.f;
    AttackIntervalOverride = 0.f;
}

ABossEnemy::ABossEnemy()
{
    PrimaryActorTick.bCanEverTick = true;

    DefaultAttackInterval = 5.f;
    bLoopPhasePatterns = true;
    MovementRadius = 900.f;
    MovementHeightOffset = 0.f;
    MovementInterpSpeed = 1.75f;
    bFacePlayer = true;

    CurrentPhaseIndex = INDEX_NONE;
    CurrentPatternIndex = 0;
    AttackTimer = 0.f;
    SummonTimer = 0.f;
    bTelegraphActive = false;
    bHasCurrentPattern = false;
    CachedMaxHP = 0.f;

    bUseBaseChase = false;
    
    LastLoggedPosition = FVector::ZeroVector;
    LastPositionLogTime = 0.f;
}

void ABossEnemy::BeginPlay()
{
    Super::BeginPlay();

    CachedMaxHP = MaxHP > 0.f ? MaxHP : FMath::Max(CurrentHP, 1.f);

    Phases.Sort([](const FBossPhaseDefinition& A, const FBossPhaseDefinition& B)
    {
        return A.HealthThreshold > B.HealthThreshold;
    });

    CacheSpawner();

    if (Phases.Num() > 0)
    {
        EnterPhase(0);
    }

    // Log da posição inicial do boss
    const FVector InitialLocation = GetActorLocation();
    LOG_ENEMIES(Warn, TEXT("[BossPosition] %s BeginPlay at location: X=%.2f, Y=%.2f, Z=%.2f"), 
        *GetName(), InitialLocation.X, InitialLocation.Y, InitialLocation.Z);
    LastLoggedPosition = InitialLocation;
    LastPositionLogTime = GetWorld()->GetTimeSeconds();

    OnBossHealthChanged.Broadcast(GetHealthFraction());
}

void ABossEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    EvaluatePhase();
    PerformMovementPattern(DeltaTime);
    HandleSummoning(DeltaTime);
    TickAttackCycle(DeltaTime);
}

void ABossEnemy::HandleDeath(bool bIsParentParam)
{
    OnBossDefeated.Broadcast(this);
    LOG_ENEMIES(Info, TEXT("Boss %s defeated"), *GetName());

    Super::HandleDeath(bIsParentParam);
}

float ABossEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (AppliedDamage > 0.f)
    {
        OnBossHealthChanged.Broadcast(GetHealthFraction());
        EvaluatePhase();
    }

    return AppliedDamage;
}

// Return copy safe for BP
FBossPhaseDefinition ABossEnemy::GetCurrentPhase() const
{
    const FBossPhaseDefinition* Phase = GetCurrentPhasePtr();
    return Phase ? *Phase : FBossPhaseDefinition();
}

void ABossEnemy::EvaluatePhase()
{
    if (Phases.Num() == 0)
    {
        return;
    }

    const float HealthFraction = GetHealthFraction();

    int32 DesiredIndex = INDEX_NONE;
    for (int32 Index = 0; Index < Phases.Num(); ++Index)
    {
        const float Threshold = FMath::Clamp(Phases[Index].HealthThreshold, 0.f, 1.f);
        if (HealthFraction <= Threshold)
        {
            DesiredIndex = Index;
        }
    }

    if (DesiredIndex == INDEX_NONE)
    {
        DesiredIndex = Phases.Num() - 1;
    }

    DesiredIndex = FMath::Clamp(DesiredIndex, 0, Phases.Num() - 1);

    if (DesiredIndex != CurrentPhaseIndex)
    {
        EnterPhase(DesiredIndex);
    }
}

void ABossEnemy::EnterPhase(int32 NewPhaseIndex)
{
    if (!Phases.IsValidIndex(NewPhaseIndex))
    {
        return;
    }

    CurrentPhaseIndex = NewPhaseIndex;
    CurrentPatternIndex = 0;
    bTelegraphActive = false;
    bHasCurrentPattern = false;

    const FBossPhaseDefinition& NewPhase = Phases[CurrentPhaseIndex];
    SummonTimer = NewPhase.SummonInterval > 0.f ? NewPhase.SummonInterval : 0.f;

    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        const float SpeedMultiplier = FMath::Max(0.1f, NewPhase.MovementSpeedMultiplier);
        Movement->MaxWalkSpeed = CurrentArchetype.BaseSpeed * SpeedMultiplier;
    }

    HandlePhaseStarted(NewPhase);
    OnBossPhaseChanged.Broadcast(CurrentPhaseIndex, NewPhase);

    LOG_ENEMIES(Info, TEXT("Boss %s entered phase %d"), *GetName(), CurrentPhaseIndex);
}

void ABossEnemy::HandlePhaseStarted(const FBossPhaseDefinition& Phase)
{
    if (Phase.AttackIntervalOverride > 0.f)
    {
        AttackTimer = Phase.AttackIntervalOverride;
    }
    else
    {
        AttackTimer = DefaultAttackInterval;
    }
}

void ABossEnemy::TickAttackCycle(float DeltaTime)
{
    if (!Phases.IsValidIndex(CurrentPhaseIndex))
    {
        return;
    }

    const FBossPhaseDefinition& Phase = Phases[CurrentPhaseIndex];
    if (Phase.AttackPatterns.Num() == 0)
    {
        return;
    }

    AttackTimer -= DeltaTime;

    if (bTelegraphActive)
    {
        if (AttackTimer <= 0.f)
        {
            FinishTelegraph();
        }
        return;
    }

    if (AttackTimer > 0.f)
    {
        return;
    }

    if (!Phase.AttackPatterns.IsValidIndex(CurrentPatternIndex))
    {
        CurrentPatternIndex = bLoopPhasePatterns ? 0 : FMath::Clamp(CurrentPatternIndex, 0, Phase.AttackPatterns.Num() - 1);
    }

    CurrentPattern = Phase.AttackPatterns[CurrentPatternIndex];
    bHasCurrentPattern = true;

    if (Phase.AttackPatterns.Num() > 0)
    {
        if (bLoopPhasePatterns)
        {
            CurrentPatternIndex = (CurrentPatternIndex + 1) % Phase.AttackPatterns.Num();
        }
        else
        {
            CurrentPatternIndex = FMath::Min(CurrentPatternIndex + 1, Phase.AttackPatterns.Num() - 1);
        }
    }

    StartTelegraph(CurrentPattern);
}

void ABossEnemy::StartTelegraph(const FBossAttackPattern& Pattern)
{
    const float TelegraphDuration = FMath::Max(0.f, Pattern.TelegraphTime);

    if (TelegraphDuration > 0.f)
    {
        bTelegraphActive = true;
        AttackTimer = TelegraphDuration;
        OnBossTelegraph.Broadcast(Pattern);
        LOG_ENEMIES(Trace, TEXT("Boss %s telegraphing pattern %s for %.2fs"), *GetName(), *Pattern.PatternName.ToString(), TelegraphDuration);
    }
    else
    {
        ExecuteAttackPattern(Pattern);
    }
}

void ABossEnemy::FinishTelegraph()
{
    bTelegraphActive = false;

    if (bHasCurrentPattern)
    {
        ExecuteAttackPattern(CurrentPattern);
    }
}

void ABossEnemy::ExecuteAttackPattern(const FBossAttackPattern& Pattern)
{
    PerformAttackPattern(Pattern);
    OnBossAttackExecuted.Broadcast(Pattern);

    const FBossPhaseDefinition* Phase = GetCurrentPhasePtr();

    float NextInterval = DefaultAttackInterval;
    if (Pattern.Cooldown > 0.f)
    {
        NextInterval = Pattern.Cooldown;
    }
    else if (Phase && Phase->AttackIntervalOverride > 0.f)
    {
        NextInterval = Phase->AttackIntervalOverride;
    }

    AttackTimer = FMath::Max(0.1f, NextInterval);
    bHasCurrentPattern = false;

    if (Pattern.bSummonsMinions)
    {
        SpawnSummonedMinions(Pattern.MinionType, Pattern.MinionCount, Pattern.MinionSpawnRadius, Pattern.MinionModifiers);
    }

    LOG_ENEMIES(Info, TEXT("Boss %s executed pattern %s"), *GetName(), *Pattern.PatternName.ToString());
}

void ABossEnemy::PerformAttackPattern(const FBossAttackPattern& Pattern)
{
    LOG_ENEMIES(Trace, TEXT("Boss %s base attack pattern %s (override to implement behaviour)"), *GetName(), *Pattern.PatternName.ToString());
}

void ABossEnemy::PerformMovementPattern(float DeltaTime)
{
    if (MovementRadius <= 0.f)
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    const float TimeSeconds = GetWorld()->GetTimeSeconds();
    const FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector DesiredLocation = PlayerLocation + FVector(FMath::Cos(TimeSeconds), FMath::Sin(TimeSeconds), 0.f) * MovementRadius;
    DesiredLocation.Z = PlayerLocation.Z + MovementHeightOffset;

    const FVector OldLocation = GetActorLocation();
    const float LerpSpeed = FMath::Max(1.f, MovementInterpSpeed);
    const FVector NewLocation = FMath::VInterpTo(OldLocation, DesiredLocation, DeltaTime, LerpSpeed);
    SetActorLocation(NewLocation);

    // Log de movimento - apenas se moveu significativamente e passou tempo suficiente
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    const float DistanceFromLastLog = FVector::Dist(NewLocation, LastLoggedPosition);
    const float TimeSinceLastLog = CurrentTime - LastPositionLogTime;
    
    if (DistanceFromLastLog > 200.f || TimeSinceLastLog > 5.f) // Log a cada 200 unidades de movimento ou a cada 5 segundos
    {
    LOG_LOOP_THROTTLE(Warn, FVazioLog::MakeLoopKey(__FUNCTION__), 1, 5.f, TEXT("[BossMovement] %s moved to: X=%.2f, Y=%.2f, Z=%.2f (Distance from last: %.2f)"), 
               *GetName(), NewLocation.X, NewLocation.Y, NewLocation.Z, DistanceFromLastLog);
        LastLoggedPosition = NewLocation;
        LastPositionLogTime = CurrentTime;
    }

    if (bFacePlayer)
    {
        const FVector Direction = (PlayerLocation - NewLocation).GetSafeNormal();
        if (!Direction.IsNearlyZero())
        {
            SetActorRotation(Direction.Rotation());
        }
    }
}

void ABossEnemy::HandleSummoning(float DeltaTime)
{
    if (!Phases.IsValidIndex(CurrentPhaseIndex))
    {
        return;
    }

    const FBossPhaseDefinition& Phase = Phases[CurrentPhaseIndex];
    if (!Phase.bEnableSummoningLoop || Phase.SummonInterval <= 0.f || Phase.SummonCount <= 0)
    {
        return;
    }

    SummonTimer -= DeltaTime;
    if (SummonTimer <= 0.f)
    {
        SummonTimer = Phase.SummonInterval;
        SpawnSummonedMinions(Phase.SummonType, Phase.SummonCount, Phase.SummonRadius, Phase.SummonModifiers);
    }
}

void ABossEnemy::SpawnSummonedMinions(FName MinionType, int32 Count, float Radius, const FEnemyInstanceModifiers& Mods)
{
    if (MinionType.IsNone() || Count <= 0)
    {
        return;
    }

    if (GetEnemyImpl() == 1)
    {
        if (UWorld* World = GetWorld())
        {
            if (UEnemyMassSpawnerSubsystem* MassSpawner = World->GetSubsystem<UEnemyMassSpawnerSubsystem>())
            {
                const FVector Origin = GetActorLocation();
                const int32 ValidCount = FMath::Max(1, Count);

                TArray<FTransform> SpawnTransforms;
                SpawnTransforms.Reserve(ValidCount);

                for (int32 Index = 0; Index < ValidCount; ++Index)
                {
                    const float Angle = (2.f * PI * Index) / ValidCount;
                    FVector Offset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Radius;
                    Offset.Z = 0.f;
                    const FVector SpawnLocation = Origin + Offset;
                    const FRotator Facing = (Origin - SpawnLocation).Rotation();
                    SpawnTransforms.Emplace(Facing.Quaternion(), SpawnLocation);
                }

                MassSpawner->SpawnEnemiesAtTransforms(MinionType, SpawnTransforms, Mods);
                return;
            }
        }

        LOG_ENEMIES(Warn, TEXT("Boss %s attempted to summon %s via Mass but spawner was unavailable"), *GetName(), *MinionType.ToString());
        return;
    }

    CacheSpawner();
    UEnemySpawnerSubsystem* Spawner = CachedSpawnerSubsystem.Get();
    if (!Spawner)
    {
        LOG_ENEMIES(Warn, TEXT("Boss %s attempted to summon %s but no spawner subsystem available"), *GetName(), *MinionType.ToString());
        return;
    }

    const FVector Origin = GetActorLocation();
    const int32 ValidCount = FMath::Max(1, Count);

    for (int32 Index = 0; Index < ValidCount; ++Index)
    {
        const float Angle = (2.f * PI * Index) / ValidCount;
        FVector Offset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Radius;
        Offset.Z = 0.f;

        const FVector SpawnLocation = Origin + Offset;
        const FTransform SpawnTransform(GetActorRotation(), SpawnLocation);

        Spawner->SpawnOne(MinionType, SpawnTransform, Mods);
    }
}

float ABossEnemy::GetHealthFraction() const
{
    const float MaxValue = FMath::Max(1.f, CachedMaxHP);
    return FMath::Clamp(CurrentHP / MaxValue, 0.f, 1.f);
}

void ABossEnemy::CacheSpawner()
{
    if (!CachedSpawnerSubsystem.IsValid())
    {
        if (UWorld* World = GetWorld())
        {
            CachedSpawnerSubsystem = World->GetSubsystem<UEnemySpawnerSubsystem>();
        }
    }
}


