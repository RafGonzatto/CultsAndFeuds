#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "EnemyConfigSharedFragment.generated.h"

/**
 * Shared configuration data bridged from legacy enemy configs into mass entities.
 */
USTRUCT()
struct VAZIOMASS_API FEnemyConfigSharedFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Enemy|Config")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Enemy|Config")
    float Speed = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Enemy|Config")
    float Damage = 10.0f;

    UPROPERTY(EditAnywhere, Category = "Enemy|Config")
    float CollisionRadius = 50.0f;

    UPROPERTY(EditAnywhere, Category = "Enemy|Rewards")
    int32 XPReward = 1;
};
