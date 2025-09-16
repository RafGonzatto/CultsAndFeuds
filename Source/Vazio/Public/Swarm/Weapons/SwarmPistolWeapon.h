#pragma once

#include "CoreMinimal.h"
#include "Swarm/Weapons/SwarmWeaponBase.h"
#include "SwarmPistolWeapon.generated.h"

class AEnemyBase;
class UStaticMeshComponent;
class ARangedProjectile;

/**
 * Simple projectile weapon that targets the nearest enemy and fires a ranged projectile.
 */
UCLASS()
class VAZIO_API ASwarmPistolWeapon : public ASwarmWeaponBase
{
    GENERATED_BODY()

public:
    ASwarmPistolWeapon();

protected:
    virtual bool PerformAttack() override;
    virtual bool CanPerformAttack() const override;

private:
    FVector GetMuzzleWorldLocation() const;

private:
    UPROPERTY(VisibleAnywhere, Category = "Weapon")
    TObjectPtr<UStaticMeshComponent> VisualMesh;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    TSubclassOf<ARangedProjectile> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float ProjectileSpeed = 1200.0f;

    mutable TWeakObjectPtr<AEnemyBase> CachedTarget;
};
