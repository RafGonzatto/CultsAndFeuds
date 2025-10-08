#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityManager.h"
#include "EnemyCombatProcessor.generated.h"

/** Processor that consumes combat requests and coordinates special attack game-thread effects. */
UCLASS()
class VAZIOMASS_API UEnemyCombatProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UEnemyCombatProcessor();

protected:
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
};
