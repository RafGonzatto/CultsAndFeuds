#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityManager.h"
#include "HAL/IConsoleManager.h"
#include "EnemyMassVisualizationProcessor.generated.h"

extern TAutoConsoleVariable<int32> GMassEnemyDebug;

/** Debug-only processor that renders simple gizmos for mass enemies. */
UCLASS()
class VAZIOMASS_API UEnemyMassVisualizationProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UEnemyMassVisualizationProcessor();

protected:
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
};
