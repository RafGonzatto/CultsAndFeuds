#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityManager.h"
#include "EnemyReplicationProcessors.generated.h"

class UMassEntitySubsystem;
class AEnemyMassReplicator;

/** Publishes server Mass entity state to a replicated actor. */
UCLASS()
class VAZIOMASS_API UEnemyReplicationServerProcessor : public UMassProcessor
{
    GENERATED_BODY()
public:
    UEnemyReplicationServerProcessor();
protected:
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
private:
    FMassEntityQuery EntityQuery;
};

/** Applies replicated snapshots into the client Mass world. */
UCLASS()
class VAZIOMASS_API UEnemyReplicationClientProcessor : public UMassProcessor
{
    GENERATED_BODY()
public:
    UEnemyReplicationClientProcessor();
protected:
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
private:
    FMassEntityQuery EntityQuery;
};
