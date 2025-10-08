#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "EnemyVisualSyncProcessor.generated.h"

/** Synchronises Mass enemy transforms with their visual proxies on the game thread. */
UCLASS()
class VAZIOMASS_API UEnemyVisualSyncProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UEnemyVisualSyncProcessor();

protected:
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
};
