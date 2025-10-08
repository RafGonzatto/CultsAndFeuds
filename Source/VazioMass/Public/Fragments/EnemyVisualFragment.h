#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "EnemyVisualFragment.generated.h"

/** Lightweight handle linking a Mass enemy entity to its visual proxy. */
USTRUCT()
struct VAZIOMASS_API FEnemyVisualHandleFragment : public FMassFragment
{
    GENERATED_BODY()

    /** Handle managed by the visualization subsystem. */
    UPROPERTY()
    int32 VisualHandle = INDEX_NONE;

    /** Cached archetype name to drive styling decisions. */
    UPROPERTY()
    FName ArchetypeName = NAME_None;
};
