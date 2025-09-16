#include "Swarm/Weapons/SwarmSpinningSawsWeapon.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Enemy/EnemyBase.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "World/Common/Player/MyCharacter.h"

ASwarmSpinningSawsWeapon::ASwarmSpinningSawsWeapon()
{
    Damage = 12.0f;
    Cooldown = 0.0f; // damage handled by TickInterval
    TickInterval = DamageInterval;
    AttackRange = OrbitRadius + SawCollisionRadius;
    bAutoActivate = true;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SawMesh(TEXT("/Engine/BasicShapes/Cylinder"));
    if (SawMesh.Succeeded())
    {
        SawMeshAsset = SawMesh.Object;
    }

    AttachmentOffset = FVector(0.0f, 0.0f, 45.0f);
}

void ASwarmSpinningSawsWeapon::BeginPlay()
{
    Super::BeginPlay();
    TickInterval = FMath::Max(0.05f, DamageInterval);
    InitializeSaws();
}

bool ASwarmSpinningSawsWeapon::CanPerformAttack() const
{
    return Super::CanPerformAttack();
}

bool ASwarmSpinningSawsWeapon::PerformAttack()
{
    return false; // damage applied via OnTickIntervalElapsed
}

void ASwarmSpinningSawsWeapon::OnWeaponTick(float DeltaSeconds)
{
    Super::OnWeaponTick(DeltaSeconds);
    UpdateSawTransforms(DeltaSeconds);
}

void ASwarmSpinningSawsWeapon::OnTickIntervalElapsed()
{
    UWorld* World = GetWorld();
    AMyCharacter* OwnerChar = GetOwningCharacter();
    if (!World || !IsValid(OwnerChar))
    {
        return;
    }

    const float CurrentTime = World->GetTimeSeconds();
    const float EffectiveInterval = FMath::Max(0.05f, DamageInterval);

    for (auto It = OverlappingEnemies.CreateIterator(); It; ++It)
    {
        TWeakObjectPtr<AEnemyBase> EnemyPtr = *It;
        AEnemyBase* Enemy = EnemyPtr.Get();
        if (!IsValid(Enemy))
        {
            LastDamageTimes.Remove(EnemyPtr);
            It.RemoveCurrent();
            continue;
        }

        float* LastDamageTime = LastDamageTimes.Find(EnemyPtr);
        const float TimeSinceDamage = LastDamageTime ? (CurrentTime - *LastDamageTime) : MAX_flt;
        if (TimeSinceDamage < EffectiveInterval)
        {
            continue;
        }

        FPointDamageEvent DamageEvent;
        DamageEvent.Damage = Damage;
        DamageEvent.ShotDirection = (Enemy->GetActorLocation() - OwnerChar->GetActorLocation()).GetSafeNormal();
        DamageEvent.HitInfo.Location = Enemy->GetActorLocation();

        Enemy->TakeDamage(Damage, DamageEvent, OwnerChar->GetController(), this);
        LastDamageTimes.Add(EnemyPtr, CurrentTime);
    }
}

void ASwarmSpinningSawsWeapon::InitializeSaws()
{
    SawColliders.Empty();
    SawMeshes.Empty();
    SawAngles.Empty();

    const int32 Count = FMath::Max(1, SawCount);
    SawColliders.Reserve(Count);
    SawMeshes.Reserve(Count);
    SawAngles.Reserve(Count);

    for (int32 Index = 0; Index < Count; ++Index)
    {
        const FString CollisionName = FString::Printf(TEXT("SawCollision_%d"), Index);
        USphereComponent* Collision = NewObject<USphereComponent>(this, FName(*CollisionName));
        Collision->InitSphereRadius(SawCollisionRadius);
        Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Collision->SetCollisionResponseToAllChannels(ECR_Overlap);
        Collision->SetGenerateOverlapEvents(true);
        Collision->SetRelativeLocation(FVector::ZeroVector);
        Collision->SetupAttachment(WeaponRoot);
        Collision->RegisterComponent();

        Collision->OnComponentBeginOverlap.AddDynamic(this, &ASwarmSpinningSawsWeapon::OnSawBeginOverlap);
        Collision->OnComponentEndOverlap.AddDynamic(this, &ASwarmSpinningSawsWeapon::OnSawEndOverlap);

        const FString MeshName = FString::Printf(TEXT("SawMesh_%d"), Index);
        UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(this, FName(*MeshName));
        Mesh->SetupAttachment(Collision);
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Mesh->SetGenerateOverlapEvents(false);
        Mesh->RegisterComponent();

        if (SawMeshAsset)
        {
            Mesh->SetStaticMesh(SawMeshAsset);
            Mesh->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.05f));
            Mesh->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
        }

        const float BaseAngle = (360.0f / Count) * Index;
        SawAngles.Add(BaseAngle);

        const float AngleRad = FMath::DegreesToRadians(BaseAngle);
        const FVector Offset(FMath::Cos(AngleRad) * OrbitRadius, FMath::Sin(AngleRad) * OrbitRadius, SawHeightOffset);
        Collision->SetRelativeLocation(Offset);

        SawColliders.Add(Collision);
        SawMeshes.Add(Mesh);
    }
}

void ASwarmSpinningSawsWeapon::UpdateSawTransforms(float DeltaSeconds)
{
    if (SawColliders.Num() == 0)
    {
        return;
    }

    const int32 Count = SawColliders.Num();
    for (int32 Index = 0; Index < Count; ++Index)
    {
        USphereComponent* Collision = SawColliders[Index];
        UStaticMeshComponent* Mesh = SawMeshes.IsValidIndex(Index) ? SawMeshes[Index] : nullptr;
        if (!Collision)
        {
            continue;
        }

        float& Angle = SawAngles[Index];
        Angle += OrbitSpeed * DeltaSeconds;
        if (Angle >= 360.0f)
        {
            Angle -= 360.0f;
        }

        const float AngleRad = FMath::DegreesToRadians(Angle);
        const FVector Offset(FMath::Cos(AngleRad) * OrbitRadius, FMath::Sin(AngleRad) * OrbitRadius, SawHeightOffset);
        Collision->SetRelativeLocation(Offset);

        if (Mesh)
        {
            Mesh->AddLocalRotation(FRotator(SawRotationSpeed * DeltaSeconds, 0.0f, 0.0f));
        }
    }
}

void ASwarmSpinningSawsWeapon::OnSawBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 /*OtherBodyIndex*/, bool /*bFromSweep*/, const FHitResult& /*SweepResult*/)
{
    if (AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor))
    {
        OverlappingEnemies.Add(Enemy);
    }
}

void ASwarmSpinningSawsWeapon::OnSawEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 /*OtherBodyIndex*/)
{
    if (AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor))
    {
        OverlappingEnemies.Remove(Enemy);
        LastDamageTimes.Remove(Enemy);
    }
}