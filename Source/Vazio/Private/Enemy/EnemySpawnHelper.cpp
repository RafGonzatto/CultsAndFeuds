#include "Enemy/EnemySpawnHelper.h"
#include "Enemy/SpawnTimeline.h"
#include "Enemy/EnemySpawnerSubsystem.h"
#include "Enemy/EnemyConfig.h"
#include "Engine/World.h"
#include "Enemy/EnemyTypes.h"
#include "Systems/EnemyMassSystem.h"
#include "Logging/VazioLogFacade.h"

USpawnTimeline* UEnemySpawnHelper::CreateTimelineFromJSON(const FString& JSONString)
{
    USpawnTimeline* Timeline = NewObject<USpawnTimeline>();
  if (Timeline->ParseFromJSON(JSONString))
  {
    LOG_ENEMIES(Info, TEXT("Created timeline with %d events"), Timeline->Events.Num());
        return Timeline;
    }
    else
    {
    LOG_ENEMIES(Error, TEXT("Failed to parse JSON for spawn timeline"));
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
        LOG_ENEMIES(Error, TEXT("StartSpawnTimeline invalid parameters"));
        return;
    }
    
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        LOG_ENEMIES(Error, TEXT("StartSpawnTimeline could not get world"));
        return;
    }

    // Ensure Mass subsystem exists prior to accessing spawner bridge
    World->GetSubsystem<UEnemyMassSystem>();
    
    UEnemySpawnerSubsystem* SpawnerSubsystem = World->GetSubsystem<UEnemySpawnerSubsystem>();
    if (!SpawnerSubsystem)
    {
        LOG_ENEMIES(Error, TEXT("StartSpawnTimeline missing EnemySpawnerSubsystem"));
        return;
    }
    
    // Make sure we have an enemy config
    if (!SpawnerSubsystem->GetEnemyConfig())
    {
        UEnemyConfig* DefaultConfig = UEnemyConfig::CreateDefaultConfig();
        SpawnerSubsystem->SetEnemyConfig(DefaultConfig);
        LOG_ENEMIES(Info, TEXT("Created and set default enemy config"));
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
