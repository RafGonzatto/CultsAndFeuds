#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HAL/CriticalSection.h"
#include "EnemyPerceptionService.generated.h"

class UWorld;

/**
 * Service responsible for gathering perception data (e.g. player targets) on the game thread.
 * Mass processors consume the cached target information without relying on UObjects per entity.
 */
UCLASS()
class VAZIOMASS_API UEnemyPerceptionService : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(UWorld* InWorld);

    /** Called on the game thread to refresh primary target information. */
    void TickPerception(float DeltaTime);

    /** Thread-safe access to the current primary target location. */
    FVector GetPrimaryTargetLocation() const;

    /** Whether the service has a valid primary target. */
    bool HasValidPrimaryTarget() const;

private:
    void UpdatePrimaryTarget();

private:
    TWeakObjectPtr<UWorld> CachedWorld;

    mutable FCriticalSection TargetLock;
    FVector CachedPrimaryTargetLocation = FVector::ZeroVector;
    bool bHasPrimaryTarget = false;
};
