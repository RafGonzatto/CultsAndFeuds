#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityManager.h"
#include "EnemyLODProcessor.generated.h"

/** Updates Mass enemy LOD fragments based on distance to the local player. */
UCLASS()
class VAZIOMASS_API UEnemyLODProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UEnemyLODProcessor();

protected:
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
};
