#include "Enemy/Types/RangedEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Enemy/EnemyTypes.h"
#include "World/Common/Projectiles/RangedProjectile.h"
#include "Logging/VazioLogFacade.h"

ARangedEnemy::ARangedEnemy()
{
    PrimaryActorTick.bCanEverTick = true;
    AttackRange = 800.f;
    FireRate = 2.f;
    OptimalDistance = 600.f;
    LastFireTime = 0.f;
    bUseBaseChase = false; // we manage movement ourselves
    ProjectileClass = ARangedProjectile::StaticClass();
}

void ARangedEnemy::BeginPlay()
{
    Super::BeginPlay();
    // Ensure we have some movement speed
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        if (Move->MaxWalkSpeed <= 1.f)
        {
            Move->MaxWalkSpeed = 300.f;
        }
    }
}

void ARangedEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!CurrentModifiers.bImmovable)
    {
        // Ensure movement component is active
        if (UCharacterMovementComponent* Move = GetCharacterMovement())
        {
            Move->Activate(true);
            if (Move->MovementMode == MOVE_None)
            {
                Move->SetMovementMode(MOVE_Walking);
            }
        }
        HandleRangedCombat(DeltaTime);
    }
}

void ARangedEnemy::HandleRangedCombat(float DeltaTime)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    const FVector MyLoc = GetActorLocation();
    const FVector PlayerLoc = PlayerPawn->GetActorLocation();
    FVector ToPlayer = PlayerLoc - MyLoc;
    ToPlayer.Z = 0.f;
    float DistanceToPlayer = ToPlayer.Size();
    FVector Direction = ToPlayer.GetSafeNormal();

    // Always face the player snapshot
    if (!Direction.IsNearlyZero())
    {
        SetActorRotation(FRotationMatrix::MakeFromX(Direction).Rotator());
    }

    // Movement logic to keep distance
    const float ApproachThresh = OptimalDistance * 0.9f; // move closer if beyond this
    const float RetreatThresh = OptimalDistance * 0.7f;  // retreat if closer than this
    const float FarThresh = OptimalDistance * 1.3f;      // slightly far

    if (DistanceToPlayer > AttackRange)
    {
        // Too far to attack: close in quickly
        AddMovementInput(Direction, 1.0f, true);
    }
    else if (DistanceToPlayer < RetreatThresh)
    {
        // Too close: back away faster
        AddMovementInput(-Direction, 0.8f, true);
    }
    else if (DistanceToPlayer > FarThresh)
    {
        // Slightly far: close in slowly
        AddMovementInput(Direction, 0.4f, true);
    }
    // else: hold position in the pocket

    // Fallback nudge if stuck
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        if (Move->Velocity.Size2D() < 1.f && !Direction.IsNearlyZero())
        {
            const FVector Nudge = Direction * 50.f * FMath::Max(0.016f, DeltaTime);
            AddActorWorldOffset(Nudge, true);
        }
    }

    // Fire logic: shoot straight toward player's position at fire time
    if (DistanceToPlayer <= AttackRange)
    {
        const float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastFireTime >= (1.f / FireRate))
        {
            FireProjectile();
            LastFireTime = CurrentTime;
        }
    }

    // Throttled debug log
    LOG_LOOP_THROTTLE(Debug, FVazioLog::MakeLoopKey("RangedCombat"), 1, 1.0f,
        TEXT("[RANGED] %s Dist=%.1f InRange=%s"), *GetName(), DistanceToPlayer, DistanceToPlayer <= AttackRange ? TEXT("Y") : TEXT("N"));
}

void ARangedEnemy::FireProjectile()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    const FVector StartLocation = GetActorLocation() + GetActorForwardVector() * 150.f + FVector(0,0,90.f);
    const FVector TargetLocation = PlayerPawn->GetActorLocation();
    FVector Dir = (TargetLocation - StartLocation);
    Dir.Z = 0.f; // keep flat if desired
    Dir = Dir.GetSafeNormal();
    const FRotator SpawnRotation = FRotationMatrix::MakeFromX(Dir).Rotator();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    TSubclassOf<AActor> ClassToSpawn = ProjectileClass;
    if (!ClassToSpawn)
    {
        ClassToSpawn = ARangedProjectile::StaticClass();
    }
    if (ClassToSpawn)
    {
        if (AActor* NewProjectile = GetWorld()->SpawnActor<AActor>(ClassToSpawn, StartLocation, SpawnRotation, SpawnParams))
        {
            if (ARangedProjectile* RP = Cast<ARangedProjectile>(NewProjectile))
            {
                RP->InitShoot(Dir, 1100.f);
                RP->Damage = CurrentArchetype.BaseDMG > 0.f ? CurrentArchetype.BaseDMG : 15.f;
            }
            LOG_ENEMIES(Info, TEXT("[RANGED] %s fired projectile"), *GetName());
        }
    }
}
