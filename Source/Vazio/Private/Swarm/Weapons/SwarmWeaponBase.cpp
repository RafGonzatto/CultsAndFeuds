#include "Swarm/Weapons/SwarmWeaponBase.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Enemy/EnemyBase.h"
#include "World/Common/Player/MyCharacter.h"

ASwarmWeaponBase::ASwarmWeaponBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    WeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot"));
    SetRootComponent(WeaponRoot);
}

void ASwarmWeaponBase::BeginPlay()
{
    Super::BeginPlay();
    ActivationCountdown = ActivationDelay;
}

void ASwarmWeaponBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!bIsActive)
    {
        return;
    }

    AMyCharacter* OwnerPtr = OwnerCharacter.Get();
    if (!IsValid(OwnerPtr))
    {
        DeactivateWeapon();
        return;
    }

    if (ActivationCountdown > 0.0f)
    {
        ActivationCountdown -= DeltaSeconds;
        if (ActivationCountdown > 0.0f)
        {
            return;
        }
    }

    OnWeaponTick(DeltaSeconds);

    if (TickInterval > 0.0f)
    {
        IntervalAccumulator += DeltaSeconds;
        if (IntervalAccumulator >= TickInterval)
        {
            IntervalAccumulator -= TickInterval;
            OnTickIntervalElapsed();
        }
    }

    if (Cooldown > 0.0f)
    {
        TimeSinceLastShot += DeltaSeconds;
        if (TimeSinceLastShot >= Cooldown && CanPerformAttack())
        {
            if (PerformAttack())
            {
                TimeSinceLastShot = 0.0f;
            }
            else
            {
                TimeSinceLastShot = Cooldown;
            }
        }
    }
}

void ASwarmWeaponBase::InitializeWeapon(AMyCharacter* InOwner)
{
    OwnerCharacter = InOwner;
    if (!IsValid(InOwner))
    {
        return;
    }

    FAttachmentTransformRules AttachRules(EAttachmentRule::KeepWorld, true);
    AttachToComponent(InOwner->GetRootComponent(), AttachRules);
    SetOwner(InOwner);

    SetActorRelativeLocation(AttachmentOffset);
    SetActorRelativeRotation(AttachmentRotation);
    WeaponRoot->SetRelativeLocation(FVector::ZeroVector);
    WeaponRoot->SetRelativeRotation(FRotator::ZeroRotator);

    ActivationCountdown = ActivationDelay;

    if (bAutoActivate)
    {
        ActivateWeapon();
    }
    else
    {
        DeactivateWeapon();
    }
}

void ASwarmWeaponBase::ActivateWeapon()
{
    ActivationCountdown = ActivationDelay;
    SetWeaponActiveInternal(true);
}

void ASwarmWeaponBase::DeactivateWeapon()
{
    SetWeaponActiveInternal(false);
}

void ASwarmWeaponBase::OnWeaponTick(float /*DeltaSeconds*/)
{
}

bool ASwarmWeaponBase::PerformAttack()
{
    return false;
}

bool ASwarmWeaponBase::CanPerformAttack() const
{
    return OwnerCharacter.IsValid();
}

void ASwarmWeaponBase::OnTickIntervalElapsed()
{
}

void ASwarmWeaponBase::ApplyUpgrade(float Value)
{
    AddDamage(Value);
}

void ASwarmWeaponBase::AddDamage(float Amount)
{
    Damage = FMath::Max(0.0f, Damage + Amount);
}

AEnemyBase* ASwarmWeaponBase::FindNearestEnemy(float MaxRange, float& OutDistance) const
{
    OutDistance = 0.0f;

    const AMyCharacter* OwnerPtr = OwnerCharacter.Get();
    if (!IsValid(OwnerPtr))
    {
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    const FVector Origin = OwnerPtr->GetActorLocation();
    const float MaxRangeSq = (MaxRange > 0.0f) ? FMath::Square(MaxRange) : MAX_flt;

    AEnemyBase* ClosestEnemy = nullptr;
    float ClosestDistSq = MaxRangeSq;

    for (TActorIterator<AEnemyBase> It(World); It; ++It)
    {
        AEnemyBase* Candidate = *It;
        if (!IsValid(Candidate))
        {
            continue;
        }

        // Replace deprecated IsPendingKill usage
        if (Candidate->IsActorBeingDestroyed() || Candidate->HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed))
        {
            continue;
        }

        if (Candidate->GetCurrentHP() <= 0.0f)
        {
            continue;
        }

        const float DistSq = FVector::DistSquared(Origin, Candidate->GetActorLocation());
        if (DistSq <= ClosestDistSq)
        {
            ClosestDistSq = DistSq;
            ClosestEnemy = Candidate;
        }
    }

    if (ClosestEnemy)
    {
        OutDistance = FMath::Sqrt(ClosestDistSq);
    }

    return ClosestEnemy;
}

void ASwarmWeaponBase::SetWeaponActiveInternal(bool bNewActive)
{
    if (bIsActive == bNewActive)
    {
        return;
    }

    bIsActive = bNewActive;
    SetActorTickEnabled(bIsActive);

    IntervalAccumulator = 0.0f;

    if (bIsActive)
    {
        TimeSinceLastShot = Cooldown;
    }
    else
    {
        TimeSinceLastShot = 0.0f;
    }
}