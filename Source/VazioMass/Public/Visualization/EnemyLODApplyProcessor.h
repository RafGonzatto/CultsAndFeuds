#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityManager.h"
#include "EnemyLODApplyProcessor.generated.h"

/**
 * Example processor applying LOD significance into visualization layer; in a final
 * system this would toggle ISM instances or select mesh groups. Here we draw debug.
 */
UCLASS()
class VAZIOMASS_API UEnemyLODApplyProcessor : public UMassProcessor
{
    GENERATED_BODY()
public:
    UEnemyLODApplyProcessor();
protected:
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
private:
    FMassEntityQuery Query;
};
