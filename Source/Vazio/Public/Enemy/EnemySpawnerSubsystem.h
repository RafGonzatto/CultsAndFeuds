#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Enemy/EnemyTypes.h"
#include "EnemySpawnerSubsystem.generated.h"

class UEnemyConfig;
class USpawnTimeline;
class AEnemyBase;

UCLASS()
class VAZIO_API UEnemySpawnerSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Enemy Spawner")
    void StartTimeline(const USpawnTimeline* Timeline, int32 Seed = 0);

    UFUNCTION(BlueprintCallable, Category = "Enemy Spawner")
    void SpawnLinear(FName Type, int32 Count, const FEnemyInstanceModifiers& Mods);

    UFUNCTION(BlueprintCallable, Category = "Enemy Spawner")
    void SpawnCircleAroundPlayer(FName Type, int32 Count, const FEnemyInstanceModifiers& Mods, float Radius = 1000.f);

    UFUNCTION(BlueprintCallable, Category = "Enemy Spawner")
    AEnemyBase* SpawnOne(FName Type, const FTransform& Transform, const FEnemyInstanceModifiers& Mods);

    UFUNCTION(BlueprintCallable, Category = "Enemy Spawner")
    void SetEnemyConfig(UEnemyConfig* Config);

    UFUNCTION(BlueprintCallable, Category = "Enemy Spawner")
    UEnemyConfig* GetEnemyConfig() const { return CurrentEnemyConfig; }

private:
    void ScheduleEvent(const FSpawnEvent& Event);
    void ExecuteSpawnEvent(const FSpawnEvent& Event);
    
    bool FindSpawnPointLinear(FVector& OutLocation);
    bool FindSpawnPointCircle(float Radius, int32 Index, int32 TotalCount, FVector& OutLocation);
    
    AEnemyBase* CreateEnemyActor(FName Type, const FTransform& Transform);
    
    UPROPERTY()
    TObjectPtr<UEnemyConfig> CurrentEnemyConfig;
    
    FRandomStream SpawnRng;
    TArray<FTimerHandle> ScheduledTimers;
    
    // Spawn parameters - VISIBLE RANGE FOR PROPER GAMEPLAY
    UPROPERTY(EditAnywhere, Category = "Spawn Settings")
    float LinearSpawnMinDistance = 200.f;
    
    UPROPERTY(EditAnywhere, Category = "Spawn Settings")
    float LinearSpawnMaxDistance = 400.f;
    
    UPROPERTY(EditAnywhere, Category = "Spawn Settings")
    float SpawnHeight = 100.f;
    
    // Enemy class mappings
    UPROPERTY(EditAnywhere, Category = "Enemy Classes")
    TMap<FName, TSubclassOf<AEnemyBase>> EnemyClasses;
    
    void InitializeEnemyClasses();
    APawn* GetPlayerPawn() const;
};
