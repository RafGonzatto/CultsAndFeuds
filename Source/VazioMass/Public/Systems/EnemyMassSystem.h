#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnemyMassSystem.generated.h"

class UEnemyMovementProcessor;
class UEnemyPerceptionProcessor;
class UEnemyAIProcessor;
class UEnemyHealthProcessor;
class UEnemyFlowFieldProcessor;
class UEnemyDamageProcessor;
class UEnemyCombatProcessor;
class UEnemyLODProcessor;
class UEnemyMassVisualizationProcessor;
class UEnemyMassController;
class UMassEntitySubsystem;
struct FEnemyDamageEvent
{
    FVector Origin = FVector::ZeroVector;
    float Radius = 0.0f;
    float Amount = 0.0f;
    bool bCritical = false;
};

DECLARE_LOG_CATEGORY_EXTERN(LogEnemyMassSystem, Log, All);

/**
 * World subsystem coordinating enemy-related mass processors, services, and controllers.
 */
UCLASS()
class VAZIOMASS_API UEnemyMassSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Returns the world the subsystem is bound to. */
    UWorld* GetWorldChecked() const;

    /** Accessor exposing the controller for processors/services. */
    UEnemyMassController* GetController() const { return MassController; }

    /** Queues a radial damage event to be processed on the Mass side. */
    UFUNCTION(BlueprintCallable, Category = "Enemy|Mass|Damage")
    void ApplyRadialDamageMass(const FVector& Origin, float Radius, float Amount, bool bCritical = false);

    /** Internal: Dequeues all pending damage events for processing. */
    void DequeuePendingDamage(TArray<FEnemyDamageEvent>& OutEvents);

private:
    /** Ensures all baseline processors exist and are configured. */
    void RegisterCoreProcessors();

private:
    TWeakObjectPtr<UMassEntitySubsystem> EntitySubsystem;

    /** Pooled command buffer shared across processors for batched operations. */
    TSharedPtr<struct FMassCommandBuffer> SharedCommandBuffer;

    /** High-level controller orchestrating domain services. */
    UPROPERTY(Transient)
    TObjectPtr<UEnemyMassController> MassController;

    /** Cached pointer to core processors to simplify configuration and re-use. */
    UPROPERTY(Transient)
    TObjectPtr<UEnemyMovementProcessor> MovementProcessor;

    UPROPERTY(Transient)
    TObjectPtr<UEnemyPerceptionProcessor> PerceptionProcessor;

    UPROPERTY(Transient)
    TObjectPtr<UEnemyAIProcessor> AIProcessor;

    UPROPERTY(Transient)
    TObjectPtr<UEnemyFlowFieldProcessor> FlowFieldProcessor;

    UPROPERTY(Transient)
    TObjectPtr<UEnemyHealthProcessor> HealthProcessor;

    UPROPERTY(Transient)
    TObjectPtr<UEnemyDamageProcessor> DamageProcessor;

    UPROPERTY(Transient)
    TObjectPtr<UEnemyCombatProcessor> CombatProcessor;

    UPROPERTY(Transient)
    TObjectPtr<UEnemyLODProcessor> LODProcessor;

    UPROPERTY(Transient)
    TObjectPtr<UEnemyMassVisualizationProcessor> VisualizationProcessor;

    /** Pending damage events submitted by gameplay systems. */
    FCriticalSection DamageEventsLock;
    TArray<FEnemyDamageEvent> PendingDamageEvents;
};
