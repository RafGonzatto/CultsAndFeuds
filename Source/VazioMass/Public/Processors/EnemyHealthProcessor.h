#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityManager.h"
#include "EnemyHealthProcessor.generated.h"

/** Ensures mass enemy health values stay within configured bounds and emits telemetry. */
UCLASS()
class VAZIOMASS_API UEnemyHealthProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UEnemyHealthProcessor();

protected:
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
};
