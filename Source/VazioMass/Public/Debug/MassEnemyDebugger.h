#pragma once

#include "CoreMinimal.h"
#include "MassEnemyDebugger.generated.h"

UCLASS()
class VAZIOMASS_API UMassEnemyDebugger : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Enemy|Debug")
    static void DebugDrawEntities(UWorld* World);
};
