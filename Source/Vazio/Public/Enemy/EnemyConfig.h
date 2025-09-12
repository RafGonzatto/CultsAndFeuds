#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Enemy/EnemyTypes.h"
#include "EnemyConfig.generated.h"

UCLASS(BlueprintType, Blueprintable)
class VAZIO_API UEnemyConfig : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Config")
    TMap<FName, FEnemyArchetype> Archetypes;

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId("EnemyConfig", GetFName());
    }

    const FEnemyArchetype* GetArchetype(FName TypeName) const
    {
        return Archetypes.Find(TypeName);
    }

    // Factory method for creating default config
    UFUNCTION(CallInEditor, Category = "Factory")
    static UEnemyConfig* CreateDefaultConfig();
};
