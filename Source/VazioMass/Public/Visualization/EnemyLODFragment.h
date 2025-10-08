#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "EnemyLODFragment.generated.h"

/** Simple, engine-agnostic LOD fragment for enemies (0..3). */
USTRUCT()
struct VAZIOMASS_API FEnemyLODLevelFragment : public FMassFragment
{
    GENERATED_BODY()

    /** 0=Highest detail, 1=Mid, 2=Low, 3=Culled/Off */
    UPROPERTY(EditAnywhere, Category="Enemy|LOD")
    int8 Level = 0;
};
