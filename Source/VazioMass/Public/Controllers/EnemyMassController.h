#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Containers/Ticker.h"
#include "EnemyMassController.generated.h"

class UEnemyMovementService;
class UEnemyPerceptionService;
class UEnemyAIService;
class UEnemyDamageService;
struct FMassCommandBuffer;
class UMassEntitySubsystem;
struct FEnemyDamageEvent;

/**
 * High-level coordinator responsible for wiring domain services and exposing them to processors.
 */
UCLASS()
class VAZIOMASS_API UEnemyMassController : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(UWorld* InWorld, UMassEntitySubsystem* InEntitySubsystem, const TSharedPtr<FMassCommandBuffer>& InCommandBuffer);

    /** Movement service shared between movement-related processors. */
    UEnemyMovementService* GetMovementService() const { return MovementService; }

    /** Perception service shared for target acquisition. */
    UEnemyPerceptionService* GetPerceptionService() const { return PerceptionService; }

    /** AI service used by behavior processors. */
    UEnemyAIService* GetAIService() const { return AIService; }

    // TODO(DEPRECATE): Damage service disabled during bring-up

    /** Called by subsystem to register tick handling for command buffer flush and perception updates. */
    void StartGameThreadTicker();
    void StopGameThreadTicker();

    /** Adds a combat event into the shared damage queue. */
    // TODO(DEPRECATE): Damage event queuing disabled during bring-up

    /** Command buffer maintained for batched entity operations. */
    const TSharedPtr<FMassCommandBuffer>& GetCommandBuffer() const { return CommandBuffer; }

private:
    void InitializeServices();

private:
    TWeakObjectPtr<UWorld> CachedWorld;
    TWeakObjectPtr<UMassEntitySubsystem> CachedEntitySubsystem;

    UPROPERTY()
    TObjectPtr<UEnemyMovementService> MovementService;

    UPROPERTY()
    TObjectPtr<UEnemyPerceptionService> PerceptionService;

    UPROPERTY()
    TObjectPtr<UEnemyAIService> AIService;

    // TODO(DEPRECATE): Damage service storage disabled
    // UPROPERTY()
    // TObjectPtr<UEnemyDamageService> DamageService;

    TSharedPtr<FMassCommandBuffer> CommandBuffer;

    FTSTicker::FDelegateHandle TickerHandle;
};
