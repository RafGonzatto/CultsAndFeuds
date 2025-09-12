#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Enemy/SpawnTimeline.h"
#include "Enemy/EnemySpawnerSubsystem.h"
#include "EnemySpawnHelper.generated.h"

UCLASS()
class VAZIO_API UEnemySpawnHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // Create a spawn timeline from JSON string
    UFUNCTION(BlueprintCallable, Category = "Enemy Spawn", CallInEditor)
    static USpawnTimeline* CreateTimelineFromJSON(const FString& JSONString);
    
    // Get example JSON for testing
    UFUNCTION(BlueprintCallable, Category = "Enemy Spawn", CallInEditor)
    static FString GetExampleJSON();
    
    // Start spawn timeline with subsystem
    UFUNCTION(BlueprintCallable, Category = "Enemy Spawn")
    static void StartSpawnTimeline(UObject* WorldContext, USpawnTimeline* Timeline, int32 Seed = 0);
    
    // Quick spawn function for testing
    UFUNCTION(BlueprintCallable, Category = "Enemy Spawn")
    static void QuickSpawnEnemies(UObject* WorldContext, const FString& JSONString, int32 Seed = 0);
};
