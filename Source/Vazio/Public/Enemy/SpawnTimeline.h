#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Enemy/EnemyTypes.h"
#include "Dom/JsonObject.h"
#include "SpawnTimeline.generated.h"

UCLASS(BlueprintType, Blueprintable)
class VAZIO_API USpawnTimeline : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Timeline")
    TArray<FSpawnEvent> Events;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Timeline")
    TArray<FBossSpawnEntry> BossEvents;

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId("SpawnTimeline", GetFName());
    }

    bool ParseFromJSON(const FString& JSONString);

private:
    void NormalizeSpawnData(const TSharedPtr<FJsonObject>& SpawnObject, FSpawnEvent& OutEvent);
    void ParseBossEvents(const TSharedPtr<FJsonObject>& RootObject);
    FEnemyInstanceModifiers ParseModifiers(const TSharedPtr<FJsonObject>& ModObject);
    void ParseSpawnObjectIntoEvent(const TSharedPtr<FJsonObject>& SpawnsObject, FSpawnEvent& OutEvent);
};


