#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityManager.h"
#include "MassCommonTypes.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "EnemyDamageProcessor.generated.h"

class UEnemyDamageService;
class UEnemyMassController;
struct FMassCommandBuffer;
class UMassEntitySubsystem;

/** Applies queued damage events to mass-based enemies and reacts to deaths. */
UCLASS()
class VAZIOMASS_API UEnemyDamageProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UEnemyDamageProcessor();

    void SetSharedCommandBuffer(const TSharedPtr<FMassCommandBuffer>& InCommandBuffer);

protected:
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    void ResolveServices(UMassEntitySubsystem* EntitySubsystem);

private:
    FMassEntityQuery EntityQuery;

    // TODO(DEPRECATE): Damage service disabled during bring-up
    // UPROPERTY()
    // TObjectPtr<UEnemyDamageService> DamageService;

    /** Shared with the controller for batched entity operations. */
    TSharedPtr<FMassCommandBuffer> CommandBuffer;

    /** Scratch buffer reused each tick to avoid allocations. */
    // TODO(DEPRECATE): Damage buffer disabled
    // TArray<FEnemyDamageEvent> PendingDamage;
};
