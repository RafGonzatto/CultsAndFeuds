#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SwarmWeaponBase.generated.h"

class AEnemyBase;
class AMyCharacter;
class USceneComponent;

/**
 * Base class for Swarm weapon actors. Handles activation, cooldown, and owner binding.
 */
UCLASS(Abstract)
class VAZIO_API ASwarmWeaponBase : public AActor
{
    GENERATED_BODY()

public:
    ASwarmWeaponBase();

    virtual void Tick(float DeltaSeconds) override;

    /** Attach weapon to owning character and set initial active state. */
    virtual void InitializeWeapon(AMyCharacter* InOwner);

    /** Enables the weapon so it can tick and fire. */
    void ActivateWeapon();

    /** Disables the weapon, halting attacks. */
    void DeactivateWeapon();

    bool IsWeaponActive() const { return bIsActive; }

    FORCEINLINE float GetDamage() const { return Damage; }
    FORCEINLINE float GetAttackRange() const { return AttackRange; }
    FORCEINLINE AMyCharacter* GetOwningCharacter() const { return OwnerCharacter.Get(); }

    /** Apply an upgrade effect (default behaviour increases damage). */
    virtual void ApplyUpgrade(float Value);

protected:
    virtual void BeginPlay() override;

    /** Called once per tick while active for continuous behaviour. */
    virtual void OnWeaponTick(float DeltaSeconds);

    /** Perform a single attack attempt when cooldown elapses. Returns true if the attack executed. */
    virtual bool PerformAttack();

    /** Allows derived classes to block firing without deactivating. */
    virtual bool CanPerformAttack() const;

    /** Called whenever TickInterval elapses while active. */
    virtual void OnTickIntervalElapsed();

    /** Helper to locate nearest enemy to the owner. */
    AEnemyBase* FindNearestEnemy(float MaxRange, float& OutDistance) const;

protected:
    void AddDamage(float Amount);

    /** Local offset applied when attached to the owner. */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FVector AttachmentOffset = FVector::ZeroVector;

    /** Local rotation applied when attached to the owner. */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FRotator AttachmentRotation = FRotator::ZeroRotator;

    UPROPERTY(VisibleAnywhere, Category = "Weapon")
    TObjectPtr<USceneComponent> WeaponRoot;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float Damage = 10.f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float Cooldown = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float AttackRange = 1200.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float TickInterval = 0.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float ActivationDelay = 0.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    bool bAutoActivate = true;

private:
    void SetWeaponActiveInternal(bool bNewActive);

    TWeakObjectPtr<AMyCharacter> OwnerCharacter;

    float TimeSinceLastShot = 0.0f;
    float IntervalAccumulator = 0.0f;
    float ActivationCountdown = 0.0f;

    bool bIsActive = false;
};