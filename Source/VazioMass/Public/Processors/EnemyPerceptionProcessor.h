#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassCommonTypes.h"
#include "MassEntityManager.h"
#include "EnemyPerceptionProcessor.generated.h"

class UEnemyPerceptionService;

/**
 * Updates enemy target fragments using perception data acquired on the game thread.
 */
UCLASS()
class VAZIOMASS_API UEnemyPerceptionProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UEnemyPerceptionProcessor();

protected:
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;

    UPROPERTY()
    TObjectPtr<UEnemyPerceptionService> PerceptionService;
};
