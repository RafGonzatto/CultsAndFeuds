#include "Enemy/SpawnTimeline.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Enemy/EnemyTypes.h"

bool USpawnTimeline::ParseFromJSON(const FString& JSONString)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JSONString);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogEnemySpawn, Error, TEXT("Failed to parse JSON string"));
        return false;
    }

    const TArray<TSharedPtr<FJsonValue>>* SpawnEventsArray;
    if (!JsonObject->TryGetArrayField(TEXT("spawnEvents"), SpawnEventsArray))
    {
        UE_LOG(LogEnemySpawn, Error, TEXT("Missing 'spawnEvents' array in JSON"));
        return false;
    }

    Events.Empty();

    for (const TSharedPtr<FJsonValue>& EventValue : *SpawnEventsArray)
    {
        TSharedPtr<FJsonObject> EventObject = EventValue->AsObject();
        if (!EventObject.IsValid())
        {
            continue;
        }

        FSpawnEvent NewEvent;
        NewEvent.TimeSeconds = EventObject->GetNumberField(TEXT("time"));

        TSharedPtr<FJsonObject> SpawnsObject = EventObject->GetObjectField(TEXT("spawns"));
        if (SpawnsObject.IsValid())
        {
            NormalizeSpawnData(SpawnsObject, NewEvent);
        }

        Events.Add(NewEvent);
    }

    UE_LOG(LogEnemySpawn, Log, TEXT("Successfully parsed %d spawn events from JSON"), Events.Num());
    return true;
}

void USpawnTimeline::NormalizeSpawnData(const TSharedPtr<FJsonObject>& SpawnObject, FSpawnEvent& OutEvent)
{
    // Parse circle spawns first
    const TArray<TSharedPtr<FJsonValue>>* CircleArray;
    if (SpawnObject->TryGetArrayField(TEXT("circle"), CircleArray))
    {
        for (const TSharedPtr<FJsonValue>& CircleValue : *CircleArray)
        {
            TSharedPtr<FJsonObject> CircleObj = CircleValue->AsObject();
            if (!CircleObj.IsValid())
            {
                continue;
            }

            FCircleSpawn CircleSpawn;
            CircleSpawn.Type = FName(*CircleObj->GetStringField(TEXT("type")));
            CircleSpawn.Count = CircleObj->GetIntegerField(TEXT("count"));
            CircleSpawn.Radius = CircleObj->GetNumberField(TEXT("radius"));
            
            CircleSpawn.Mods = ParseModifiers(CircleObj);
            
            OutEvent.Circles.Add(CircleSpawn);
        }
    }

    // Parse linear spawns
    for (const auto& Pair : SpawnObject->Values)
    {
        if (Pair.Key == TEXT("circle"))
        {
            continue; // Already processed
        }

        FName EnemyType = FName(*Pair.Key);
        
        // Handle different value types
        if (Pair.Value->Type == EJson::Number)
        {
            // Simple numeric format: "EnemyType": 5
            FTypeCount TypeCount;
            TypeCount.Type = EnemyType;
            TypeCount.Count = Pair.Value->AsNumber();
            OutEvent.Linear.Add(TypeCount);
        }
        else if (Pair.Value->Type == EJson::Object)
        {
            // Object format: "EnemyType": {"count": 5, "big": true}
            TSharedPtr<FJsonObject> EnemyObj = Pair.Value->AsObject();
            FTypeCount TypeCount;
            TypeCount.Type = EnemyType;
            TypeCount.Count = EnemyObj->GetIntegerField(TEXT("count"));
            TypeCount.Mods = ParseModifiers(EnemyObj);
            OutEvent.Linear.Add(TypeCount);
        }
        else if (Pair.Value->Type == EJson::Array)
        {
            // Array format: "EnemyType": [3, {"count": 5, "big": true}]
            const TArray<TSharedPtr<FJsonValue>>* EnemyArray = &Pair.Value->AsArray();
            
            FTypeCount CombinedTypeCount;
            CombinedTypeCount.Type = EnemyType;
            CombinedTypeCount.Count = 0;
            
            for (const TSharedPtr<FJsonValue>& ArrayValue : *EnemyArray)
            {
                if (ArrayValue->Type == EJson::Number)
                {
                    CombinedTypeCount.Count += ArrayValue->AsNumber();
                }
                else if (ArrayValue->Type == EJson::Object)
                {
                    TSharedPtr<FJsonObject> ArrayObj = ArrayValue->AsObject();
                    CombinedTypeCount.Count += ArrayObj->GetIntegerField(TEXT("count"));
                    
                    // Apply modifiers (OR logic - any true modifier wins)
                    FEnemyInstanceModifiers NewMods = ParseModifiers(ArrayObj);
                    CombinedTypeCount.Mods.bBig = CombinedTypeCount.Mods.bBig || NewMods.bBig;
                    CombinedTypeCount.Mods.bImmovable = CombinedTypeCount.Mods.bImmovable || NewMods.bImmovable;
                    if (NewMods.DissolveSeconds > 0.f)
                    {
                        CombinedTypeCount.Mods.DissolveSeconds = NewMods.DissolveSeconds;
                    }
                }
            }
            
            OutEvent.Linear.Add(CombinedTypeCount);
        }
    }
}

FEnemyInstanceModifiers USpawnTimeline::ParseModifiers(const TSharedPtr<FJsonObject>& ModObject)
{
    FEnemyInstanceModifiers Mods;
    
    if (ModObject.IsValid())
    {
        ModObject->TryGetBoolField(TEXT("big"), Mods.bBig);
        ModObject->TryGetBoolField(TEXT("immovable"), Mods.bImmovable);
        ModObject->TryGetNumberField(TEXT("dissolve"), Mods.DissolveSeconds);
    }
    
    return Mods;
}
