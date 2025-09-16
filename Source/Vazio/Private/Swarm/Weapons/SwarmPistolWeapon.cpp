#include "Swarm/Weapons/SwarmPistolWeapon.h"

#include "Components/StaticMeshComponent.h"
#include "Enemy/EnemyBase.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "World/Common/Player/MyCharacter.h"
#include "World/Common/Projectiles/RangedProjectile.h"

ASwarmPistolWeapon::ASwarmPistolWeapon()
{
    Damage = 15.0f;
    Cooldown = 0.5f;
    AttackRange = 1600.0f;
    ProjectileSpeed = 1500.0f;
    bAutoActivate = true;

    VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PistolMesh"));
    VisualMesh->SetupAttachment(WeaponRoot);
    VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    VisualMesh->SetGenerateOverlapEvents(false);
    VisualMesh->SetRelativeLocation(FVector::ZeroVector);
    VisualMesh->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone"));
    if (ConeMesh.Succeeded())
    {
        VisualMesh->SetStaticMesh(ConeMesh.Object);
        VisualMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.7f));
    }

    ProjectileClass = ARangedProjectile::StaticClass();

    AttachmentOffset = FVector(30.0f, 15.0f, 55.0f);
    AttachmentRotation = FRotator::ZeroRotator;
}

bool ASwarmPistolWeapon::CanPerformAttack() const
{
    if (!Super::CanPerformAttack())
    {
        CachedTarget.Reset();
        return false;
    }

    float Distance = 0.0f;
    AEnemyBase* Target = FindNearestEnemy(GetAttackRange(), Distance);
    CachedTarget = Target;
    return IsValid(Target);
}

bool ASwarmPistolWeapon::PerformAttack()
{
    AMyCharacter* OwnerChar = GetOwningCharacter();
    if (!IsValid(OwnerChar))
    {
        CachedTarget.Reset();
        return false;
    }

    AEnemyBase* Target = CachedTarget.Get();
    float Distance = 0.0f;

    if (!IsValid(Target))
    {
        Target = FindNearestEnemy(GetAttackRange(), Distance);
    }
    else
    {
        Distance = FVector::Dist(OwnerChar->GetActorLocation(), Target->GetActorLocation());
        if (Distance > GetAttackRange())
        {
            Target = nullptr;
        }
    }

    CachedTarget.Reset();

    if (!IsValid(Target))
    {
        return false;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    const FVector SpawnLocation = GetMuzzleWorldLocation();
    FVector AimLocation = Target->GetActorLocation();
    if (const UCapsuleComponent* Capsule = Target->GetCapsuleComponent())
    {
        AimLocation.Z = Target->GetActorLocation().Z + Capsule->GetScaledCapsuleHalfHeight() * 0.5f;
    }
    FVector Direction = (AimLocation - SpawnLocation).GetSafeNormal();

    if (Direction.IsNearlyZero())
    {
        AimLocation = Target->GetActorLocation();
        Direction = (AimLocation - SpawnLocation).GetSafeNormal();
    }

    if (Direction.IsNearlyZero())
    {
        return false;
    }

    const FRotator SpawnRotation = Direction.Rotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwnerChar;
    SpawnParams.Instigator = OwnerChar;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // Avoid ambiguous ternary between TSubclassOf and UClass*
    TSubclassOf<ARangedProjectile> ClassToSpawn = ProjectileClass;
    if (!ClassToSpawn)
    {
        ClassToSpawn = ARangedProjectile::StaticClass();
    }

    ARangedProjectile* Projectile = World->SpawnActor<ARangedProjectile>(ClassToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
    if (!Projectile)
    {
        return false;
    }

    Projectile->Damage = Damage;
    Projectile->InitShoot(Direction, ProjectileSpeed);
    Projectile->SetOwner(OwnerChar);

    return true;
}

FVector ASwarmPistolWeapon::GetMuzzleWorldLocation() const
{
    if (VisualMesh)
    {
        const FVector ForwardOffset = VisualMesh->GetForwardVector() * 30.0f;
        return VisualMesh->GetComponentLocation() + ForwardOffset;
    }

    if (const AMyCharacter* OwnerChar = GetOwningCharacter())
    {
        return OwnerChar->GetActorLocation() + FVector(40.0f, 0.0f, 60.0f);
    }

    return GetActorLocation();
}