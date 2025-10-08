#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyMassReplicator.generated.h"

USTRUCT(BlueprintType)
struct VAZIOMASS_API FEnemyMassNetDatum
{
    GENERATED_BODY()

    UPROPERTY()
    int32 NetID = INDEX_NONE;

    UPROPERTY()
    int32 ArchetypeID = INDEX_NONE;

    UPROPERTY()
    FVector_NetQuantize10 Location = FVector::ZeroVector;

    UPROPERTY()
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY()
    float Health = 0.0f;
};

/**
 * Replicated bridge actor carrying the current snapshot of enemy Mass entities.
 */
UCLASS()
class VAZIOMASS_API AEnemyMassReplicator : public AActor
{
    GENERATED_BODY()

public:
    AEnemyMassReplicator();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** Server populates this; clients read it. */
    UPROPERTY(Replicated)
    TArray<FEnemyMassNetDatum> NetStates;
};
