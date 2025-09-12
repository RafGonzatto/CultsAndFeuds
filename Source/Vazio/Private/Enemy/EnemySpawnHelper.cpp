#include "Enemy/EnemySpawnHelper.h"
#include "Enemy/SpawnTimeline.h"
#include "Enemy/EnemySpawnerSubsystem.h"
#include "Enemy/EnemyConfig.h"
#include "Engine/World.h"
#include "Enemy/EnemyTypes.h"

USpawnTimeline* UEnemySpawnHelper::CreateTimelineFromJSON(const FString& JSONString)
{
    USpawnTimeline* Timeline = NewObject<USpawnTimeline>();
    if (Timeline->ParseFromJSON(JSONString))
    {
        UE_LOG(LogEnemySpawn, Log, TEXT("Successfully created timeline with %d events"), Timeline->Events.Num());
        return Timeline;
    }
    else
    {
        UE_LOG(LogEnemySpawn, Error, TEXT("Failed to parse JSON for spawn timeline"));
        return nullptr;
    }
}

FString UEnemySpawnHelper::GetExampleJSON()
{
    return TEXT(R"({
  "spawnEvents": [
    {
      "time": 0,
      "spawns": {
        "SplitterSlime": 10,
        "AuraEnemy": [ 3, { "count": 5, "big": true } ],
        "circle": [
          { "type": "HeavyEnemy", "count": 8, "big": true, "immovable": true, "dissolve": 30, "radius": 900 },
          { "type": "RangedEnemy", "count": 8, "radius": 1200 }
        ]
      }
    },
    { 
      "time": 5, 
      "spawns": { 
        "HeavyEnemy": 10,
        "GoldEnemy": 3
      } 
    },
    { 
      "time": 10, 
      "spawns": { 
        "NormalEnemy": { "count": 15, "big": false },
        "DashEnemy": 5,
        "circle": [
          { "type": "AuraEnemy", "count": 4, "radius": 800 }
        ]
      } 
    },
    {
      "time": 20,
      "spawns": {
        "SplitterSlime": { "count": 5, "big": true },
        "GoldEnemy": [ 2, { "count": 3, "dissolve": 15 } ],
        "circle": [
          { "type": "HeavyEnemy", "count": 12, "immovable": true, "radius": 1500 }
        ]
      }
    }
  ]
})");
}

void UEnemySpawnHelper::StartSpawnTimeline(UObject* WorldContext, USpawnTimeline* Timeline, int32 Seed)
{
    if (!WorldContext || !Timeline)
    {
        UE_LOG(LogEnemySpawn, Error, TEXT("StartSpawnTimeline: Invalid parameters"));
        return;
    }
    
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        UE_LOG(LogEnemySpawn, Error, TEXT("StartSpawnTimeline: Could not get world from context"));
        return;
    }
    
    UEnemySpawnerSubsystem* SpawnerSubsystem = World->GetSubsystem<UEnemySpawnerSubsystem>();
    if (!SpawnerSubsystem)
    {
        UE_LOG(LogEnemySpawn, Error, TEXT("StartSpawnTimeline: Could not get EnemySpawnerSubsystem"));
        return;
    }
    
    // Make sure we have an enemy config
    if (!SpawnerSubsystem->GetEnemyConfig())
    {
        UEnemyConfig* DefaultConfig = UEnemyConfig::CreateDefaultConfig();
        SpawnerSubsystem->SetEnemyConfig(DefaultConfig);
        UE_LOG(LogEnemySpawn, Log, TEXT("Created and set default enemy config"));
    }
    
    SpawnerSubsystem->StartTimeline(Timeline, Seed);
}

void UEnemySpawnHelper::QuickSpawnEnemies(UObject* WorldContext, const FString& JSONString, int32 Seed)
{
    if (USpawnTimeline* Timeline = CreateTimelineFromJSON(JSONString))
    {
        StartSpawnTimeline(WorldContext, Timeline, Seed);
    }
}
