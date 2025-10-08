#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassCommonTypes.h"
#include "MassEntityManager.h"
#include "EnemyMovementProcessor.generated.h"

class UEnemyMovementService;

/**
 * Handles kinematic integration for mass-based enemy agents.
 */
UCLASS()
class VAZIOMASS_API UEnemyMovementProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UEnemyMovementProcessor();

protected:
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    /** Mass query fetching agent and target fragments for efficient SIMD iterations. */
    FMassEntityQuery EntityQuery;

    /** Maximum steering force applied per tick. */
    UPROPERTY(EditAnywhere, Category = "Enemy|Movement")
    float MaxSteeringForce = 1200.0f;

    /** Movement service housing reusable math helpers. */
    UPROPERTY()
    TObjectPtr<UEnemyMovementService> MovementService;
};
