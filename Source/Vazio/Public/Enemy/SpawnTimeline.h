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

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId("SpawnTimeline", GetFName());
    }

    // Parse JSON data into spawn events
    bool ParseFromJSON(const FString& JSONString);

private:
    void NormalizeSpawnData(const TSharedPtr<FJsonObject>& SpawnObject, FSpawnEvent& OutEvent);
    FEnemyInstanceModifiers ParseModifiers(const TSharedPtr<FJsonObject>& ModObject);
};
