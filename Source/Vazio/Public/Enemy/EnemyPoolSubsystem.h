#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnemyPoolSubsystem.generated.h"

class AEnemyBase;

UCLASS()
class VAZIO_API UEnemyPoolSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Enemy Pool")
    AEnemyBase* GetFromPool(FName EnemyType, TSubclassOf<AEnemyBase> EnemyClass);

    UFUNCTION(BlueprintCallable, Category = "Enemy Pool")
    void ReturnToPool(AEnemyBase* Enemy);

    UFUNCTION(BlueprintCallable, Category = "Enemy Pool")
    void ClearPool();

private:
    TMap<FName, TArray<AEnemyBase*>> PooledEnemies;
    
    TMap<AEnemyBase*, FName> ActiveEnemies;
    
    UPROPERTY(EditAnywhere, Category = "Pool Settings")
    int32 MaxPoolSizePerType = 50;
    
    void PrepoolEnemies();
};
