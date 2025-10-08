#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassCommonTypes.h"
#include "MassEntityManager.h"
#include "EnemyAIProcessor.generated.h"

class UEnemyAIService;

/**
 * Computes high-level AI commands (desired direction/speed) using perception data.
 */
UCLASS()
class VAZIOMASS_API UEnemyAIProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UEnemyAIProcessor();

protected:
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;

    UPROPERTY()
    TObjectPtr<UEnemyAIService> AIService;
};
