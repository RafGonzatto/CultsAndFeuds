#include "Enemy/SpawnTimeline.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/Engine.h"

bool USpawnTimeline::ParseFromJSON(const FString& JSONString)
{
    TSharedPtr<FJsonObject> RootObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JSONString);

    if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON for SpawnTimeline"));
        return false;
    }

    // Clear existing data
    Events.Empty();
    BossEvents.Empty();

    // Parse regular spawn events from "spawnEvents" array
    const TArray<TSharedPtr<FJsonValue>>* SpawnEventsArray = nullptr;
    if (RootObject->TryGetArrayField(TEXT("spawnEvents"), SpawnEventsArray))
    {
        for (const auto& EventValue : *SpawnEventsArray)
        {
            const TSharedPtr<FJsonObject>& EventObject = EventValue->AsObject();
            if (EventObject.IsValid())
            {
                FSpawnEvent NewEvent;
                NewEvent.TimeSeconds = EventObject->GetNumberField(TEXT("time"));
                
                // Parse the "spawns" object within each event
                const TSharedPtr<FJsonObject>* SpawnsObject = nullptr;
                if (EventObject->TryGetObjectField(TEXT("spawns"), SpawnsObject))
                {
                    ParseSpawnObjectIntoEvent(*SpawnsObject, NewEvent);
                    // Only add the event if it has spawns
                    if (NewEvent.Linear.Num() > 0 || NewEvent.Circles.Num() > 0)
                    {
                        Events.Add(NewEvent);
                        UE_LOG(LogEnemySpawn, Log, TEXT("Added spawn event at time %.2f with %d linear and %d circle spawns"), 
                               NewEvent.TimeSeconds, NewEvent.Linear.Num(), NewEvent.Circles.Num());
                    }
                }
            }
        }
    }

    // Parse boss events
    ParseBossEvents(RootObject);

    UE_LOG(LogTemp, Log, TEXT("Successfully parsed SpawnTimeline with %d regular events and %d boss events"), 
           Events.Num(), BossEvents.Num());

    return true;
}

void USpawnTimeline::NormalizeSpawnData(const TSharedPtr<FJsonObject>& SpawnObject, FSpawnEvent& OutEvent)
{
    // Parse basic timing
    OutEvent.TimeSeconds = SpawnObject->GetNumberField(TEXT("time"));

    // Parse enemy type
    FString EnemyTypeName = SpawnObject->GetStringField(TEXT("enemyType"));
    OutEvent.EnemyType = FName(*EnemyTypeName);

    // Parse count
    OutEvent.Count = FMath::Max(1, (int32)SpawnObject->GetNumberField(TEXT("count")));

    // Parse radius (optional)
    OutEvent.SpawnRadius = SpawnObject->GetNumberField(TEXT("radius"));
    if (OutEvent.SpawnRadius <= 0.0f)
    {
        OutEvent.SpawnRadius = 500.0f; // Default radius
    }

    // Parse modifiers (optional)
    const TSharedPtr<FJsonObject>* ModifiersObject = nullptr;
    if (SpawnObject->TryGetObjectField(TEXT("modifiers"), ModifiersObject))
    {
        OutEvent.Modifiers = ParseModifiers(*ModifiersObject);
    }
}

void USpawnTimeline::ParseBossEvents(const TSharedPtr<FJsonObject>& RootObject)
{
    const TArray<TSharedPtr<FJsonValue>>* BossArray = nullptr;
    if (!RootObject->TryGetArrayField(TEXT("bosses"), BossArray))
    {
        return;
    }

    for (const auto& BossValue : *BossArray)
    {
        const TSharedPtr<FJsonObject>& BossObject = BossValue->AsObject();
        if (!BossObject.IsValid())
        {
            continue;
        }

        FBossSpawnEntry BossEntry;
        BossEntry.TriggerTime = BossObject->GetNumberField(TEXT("time"));
        
        FString BossTypeName = BossObject->GetStringField(TEXT("bossType"));
        BossEntry.BossType = FName(*BossTypeName);

        BossEntry.WarningDuration = BossObject->GetNumberField(TEXT("warningDuration"));
        if (BossEntry.WarningDuration <= 0.0f)
        {
            BossEntry.WarningDuration = 3.0f; // Default warning duration
        }

        BossEntry.bPauseRegularSpawns = BossObject->GetBoolField(TEXT("pauseRegularSpawns"));

        // Parse boss modifiers (optional)
        const TSharedPtr<FJsonObject>* ModifiersObject = nullptr;
        if (BossObject->TryGetObjectField(TEXT("modifiers"), ModifiersObject))
        {
            BossEntry.BossModifiers = ParseModifiers(*ModifiersObject);
        }

        BossEvents.Add(BossEntry);
    }
}

FEnemyInstanceModifiers USpawnTimeline::ParseModifiers(const TSharedPtr<FJsonObject>& ModObject)
{
    FEnemyInstanceModifiers Modifiers;

    // Parse health multiplier
    Modifiers.HealthMultiplier = ModObject->GetNumberField(TEXT("healthMultiplier"));
    if (Modifiers.HealthMultiplier <= 0.0f)
    {
        Modifiers.HealthMultiplier = 1.0f;
    }

    // Parse damage multiplier
    Modifiers.DamageMultiplier = ModObject->GetNumberField(TEXT("damageMultiplier"));
    if (Modifiers.DamageMultiplier <= 0.0f)
    {
        Modifiers.DamageMultiplier = 1.0f;
    }

    // Parse speed multiplier
    Modifiers.SpeedMultiplier = ModObject->GetNumberField(TEXT("speedMultiplier"));
    if (Modifiers.SpeedMultiplier <= 0.0f)
    {
        Modifiers.SpeedMultiplier = 1.0f;
    }

    // Parse scale multiplier
    Modifiers.ScaleMultiplier = ModObject->GetNumberField(TEXT("scaleMultiplier"));
    if (Modifiers.ScaleMultiplier <= 0.0f)
    {
        Modifiers.ScaleMultiplier = 1.0f;
    }

    // Parse reward multiplier
    Modifiers.RewardMultiplier = ModObject->GetNumberField(TEXT("rewardMultiplier"));
    if (Modifiers.RewardMultiplier <= 0.0f)
    {
        Modifiers.RewardMultiplier = 1.0f;
    }

    return Modifiers;
}

void USpawnTimeline::ParseSpawnObjectIntoEvent(const TSharedPtr<FJsonObject>& SpawnsObject, FSpawnEvent& OutEvent)
{
    // Parse each enemy type in the spawns object and populate Linear/Circles arrays
    for (const auto& Pair : SpawnsObject->Values)
    {
        const FString& EnemyTypeName = Pair.Key;
        const TSharedPtr<FJsonValue>& EnemyValue = Pair.Value;
        
        if (EnemyTypeName == TEXT("circle"))
        {
            // Handle circle spawns array
            if (EnemyValue->Type == EJson::Array)
            {
                const TArray<TSharedPtr<FJsonValue>>& CircleArray = EnemyValue->AsArray();
                for (const auto& CircleValue : CircleArray)
                {
                    if (const TSharedPtr<FJsonObject>& CircleObject = CircleValue->AsObject())
                    {
                        FCircleSpawn CircleSpawn;
                        CircleSpawn.Type = FName(*CircleObject->GetStringField(TEXT("type")));
                        CircleSpawn.Count = FMath::Max(1, (int32)CircleObject->GetNumberField(TEXT("count")));
                        CircleSpawn.Radius = CircleObject->GetNumberField(TEXT("radius"));
                        if (CircleSpawn.Radius <= 0.0f)
                        {
                            CircleSpawn.Radius = 500.0f;
                        }
                        OutEvent.Circles.Add(CircleSpawn);
                    }
                }
            }
            continue;
        }
        
        // Handle linear spawns - create FTypeCount for Linear array
        FTypeCount TypeCount;
        TypeCount.Type = FName(*EnemyTypeName);
        
        if (EnemyValue->Type == EJson::Number)
        {
            // Simple count: "NormalEnemy": 3
            TypeCount.Count = FMath::Max(1, (int32)EnemyValue->AsNumber());
        }
        else if (EnemyValue->Type == EJson::Object)
        {
            // Complex object: "NormalEnemy": { "count": 3, "big": false }
            const TSharedPtr<FJsonObject>& EnemyObject = EnemyValue->AsObject();
            TypeCount.Count = FMath::Max(1, (int32)EnemyObject->GetNumberField(TEXT("count")));
            
            // Parse modifiers if present
            const TSharedPtr<FJsonObject>* ModifiersObject = nullptr;
            if (EnemyObject->TryGetObjectField(TEXT("modifiers"), ModifiersObject))
            {
                TypeCount.Mods = ParseModifiers(*ModifiersObject);
            }
        }
        
        // Add to Linear array (which ExecuteSpawnEvent expects)
        OutEvent.Linear.Add(TypeCount);
        
        UE_LOG(LogEnemySpawn, Verbose, TEXT("Added Linear spawn: %s x%d"), *TypeCount.Type.ToString(), TypeCount.Count);
    }
}
