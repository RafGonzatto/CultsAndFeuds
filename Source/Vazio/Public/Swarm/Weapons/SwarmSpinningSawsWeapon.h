#pragma once

#include "CoreMinimal.h"
#include "Swarm/Weapons/SwarmWeaponBase.h"
#include "SwarmSpinningSawsWeapon.generated.h"

class AEnemyBase;
class USphereComponent;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * Weapon that spawns orbiting saws dealing periodic damage to enemies on contact.
 */
UCLASS()
class VAZIO_API ASwarmSpinningSawsWeapon : public ASwarmWeaponBase
{
    GENERATED_BODY()

public:
    ASwarmSpinningSawsWeapon();

protected:
    virtual void BeginPlay() override;
    virtual void OnWeaponTick(float DeltaSeconds) override;
    virtual void OnTickIntervalElapsed() override;
    virtual bool CanPerformAttack() const override;
    virtual bool PerformAttack() override;

private:
    void InitializeSaws();
    void UpdateSawTransforms(float DeltaSeconds);

    UFUNCTION()
    void OnSawBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnSawEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Saws")
    int32 SawCount = 3;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Saws")
    float OrbitRadius = 140.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Saws")
    float OrbitSpeed = 90.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Saws")
    float SawRotationSpeed = 360.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Saws")
    float SawCollisionRadius = 55.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Saws")
    float SawHeightOffset = 0.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Saws")
    float DamageInterval = 0.25f;

    UPROPERTY()
    TObjectPtr<UStaticMesh> SawMeshAsset;

    UPROPERTY(Transient)
    TArray<TObjectPtr<USphereComponent>> SawColliders;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UStaticMeshComponent>> SawMeshes;

    TArray<float> SawAngles;

    TSet<TWeakObjectPtr<AEnemyBase>> OverlappingEnemies;
    TMap<TWeakObjectPtr<AEnemyBase>, float> LastDamageTimes;
};