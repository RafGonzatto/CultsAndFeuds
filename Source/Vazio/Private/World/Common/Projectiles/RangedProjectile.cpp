#include "World/Common/Projectiles/RangedProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

ARangedProjectile::ARangedProjectile()
{
    PrimaryActorTick.bCanEverTick = true;

    VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
    RootComponent = VisualMesh;
    VisualMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    VisualMesh->SetCollisionObjectType(ECC_WorldDynamic);
    VisualMesh->SetCollisionResponseToAllChannels(ECR_Block);
    VisualMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    VisualMesh->SetNotifyRigidBodyCollision(true);
    VisualMesh->SetGenerateOverlapEvents(true);
    VisualMesh->OnComponentHit.AddDynamic(this, &ARangedProjectile::OnHit);
    VisualMesh->OnComponentBeginOverlap.AddDynamic(this, &ARangedProjectile::OnBeginOverlap);

    // Assign a visible default mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMesh.Succeeded())
    {
        VisualMesh->SetStaticMesh(SphereMesh.Object);
        VisualMesh->SetWorldScale3D(FVector(0.35f));
    }

    // Add a bright point light for visibility
    UPointLightComponent* Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
    Light->SetupAttachment(VisualMesh);
    Light->Intensity = 12000.f;
    Light->AttenuationRadius = 450.f;
    Light->bUseInverseSquaredFalloff = false;
    Light->LightColor = FColor::Yellow;

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->InitialSpeed = 1000.f;
    ProjectileMovement->MaxSpeed = 1000.f;
    ProjectileMovement->ProjectileGravityScale = 0.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
}

void ARangedProjectile::BeginPlay()
{
    Super::BeginPlay();
    // Ignore owner collision to prevent immediate self-hit
    if (AActor* OwningActor = GetOwner())
    {
        VisualMesh->IgnoreActorWhenMoving(OwningActor, true);
    }
    if (LifeSeconds > 0.f)
    {
        SetLifeSpan(LifeSeconds);
    }
}

void ARangedProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ARangedProjectile::InitShoot(const FVector& Dir, float Speed)
{
    FVector ShootDir = Dir.GetSafeNormal();
    ProjectileMovement->Velocity = ShootDir * Speed;
}

void ARangedProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (OtherActor && OtherActor != this && OtherComp)
    {
        if (OtherActor == GetOwner())
        {
            return; // still ignore owner
        }
        // Optionally apply damage
        if (Damage > 0.f)
        {
            UGameplayStatics::ApplyPointDamage(OtherActor, Damage, GetVelocity().GetSafeNormal(), Hit, GetInstigatorController(), this, nullptr);
        }
        Destroy();
    }
}

void ARangedProjectile::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this && OtherComp)
    {
        if (OtherActor == GetOwner())
        {
            return;
        }
        if (Damage > 0.f)
        {
            const FVector Dir = GetVelocity().GetSafeNormal();
            UGameplayStatics::ApplyPointDamage(OtherActor, Damage, Dir, SweepResult, GetInstigatorController(), this, nullptr);
        }
        Destroy();
    }
}