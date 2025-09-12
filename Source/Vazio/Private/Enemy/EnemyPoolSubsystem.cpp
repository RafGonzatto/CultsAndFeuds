#include "Enemy/EnemyPoolSubsystem.h"
#include "Enemy/EnemyBase.h"
#include "Engine/World.h"
#include "Enemy/EnemyTypes.h"

void UEnemyPoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    PrepoolEnemies();
    
    UE_LOG(LogEnemySpawn, Log, TEXT("EnemyPoolSubsystem initialized"));
}

void UEnemyPoolSubsystem::Deinitialize()
{
    ClearPool();
    Super::Deinitialize();
}

AEnemyBase* UEnemyPoolSubsystem::GetFromPool(FName EnemyType, TSubclassOf<AEnemyBase> EnemyClass)
{
    if (!EnemyClass)
    {
        UE_LOG(LogEnemySpawn, Error, TEXT("GetFromPool: Invalid enemy class for type %s"), *EnemyType.ToString());
        return nullptr;
    }
    
    TArray<AEnemyBase*>* TypePool = PooledEnemies.Find(EnemyType);
    if (TypePool && TypePool->Num() > 0)
    {
        // Get from pool
        AEnemyBase* PooledEnemy = (*TypePool)[TypePool->Num() - 1];
        TypePool->RemoveAt(TypePool->Num() - 1);
        
        if (IsValid(PooledEnemy))
        {
            // Reactivate the enemy
            PooledEnemy->SetActorHiddenInGame(false);
            PooledEnemy->SetActorEnableCollision(true);
            PooledEnemy->SetActorTickEnabled(true);
            
            // Track as active
            ActiveEnemies.Add(PooledEnemy, EnemyType);
            
            UE_LOG(LogEnemySpawn, VeryVerbose, TEXT("Retrieved %s from pool (pool size now: %d)"), 
                   *EnemyType.ToString(), TypePool->Num());
            
            return PooledEnemy;
        }
        else
        {
            // Remove invalid entry
            TypePool->RemoveAt(TypePool->Num() - 1);
        }
    }
    
    // Create new enemy if pool is empty
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AEnemyBase* NewEnemy = GetWorld()->SpawnActor<AEnemyBase>(EnemyClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    if (NewEnemy)
    {
        ActiveEnemies.Add(NewEnemy, EnemyType);
        UE_LOG(LogEnemySpawn, VeryVerbose, TEXT("Created new %s (pool was empty)"), *EnemyType.ToString());
    }
    
    return NewEnemy;
}

void UEnemyPoolSubsystem::ReturnToPool(AEnemyBase* Enemy)
{
    if (!IsValid(Enemy))
    {
        return;
    }
    
    FName* EnemyType = ActiveEnemies.Find(Enemy);
    if (!EnemyType)
    {
        UE_LOG(LogEnemySpawn, Warning, TEXT("Attempted to return untracked enemy to pool: %s"), *Enemy->GetName());
        Enemy->Destroy();
        return;
    }
    
    // Remove from active tracking
    FName TypeName = *EnemyType;
    ActiveEnemies.Remove(Enemy);
    
    // Check if pool has space
    TArray<AEnemyBase*>& TypePool = PooledEnemies.FindOrAdd(TypeName);
    if (TypePool.Num() >= MaxPoolSizePerType)
    {
        // Pool is full, destroy the enemy
        Enemy->Destroy();
        UE_LOG(LogEnemySpawn, VeryVerbose, TEXT("Pool full for %s, destroying enemy"), *TypeName.ToString());
        return;
    }
    
    // Reset enemy state
    Enemy->SetActorHiddenInGame(true);
    Enemy->SetActorEnableCollision(false);
    Enemy->SetActorTickEnabled(false);
    Enemy->SetActorLocation(FVector::ZeroVector);
    Enemy->SetActorRotation(FRotator::ZeroRotator);
    
    // Clear any timers or states
    // This would be enemy-specific cleanup
    
    // Add to pool
    TypePool.Add(Enemy);
    
    UE_LOG(LogEnemySpawn, VeryVerbose, TEXT("Returned %s to pool (pool size now: %d)"), 
           *TypeName.ToString(), TypePool.Num());
}

void UEnemyPoolSubsystem::ClearPool()
{
    // Destroy all pooled enemies
    for (auto& PoolPair : PooledEnemies)
    {
        for (AEnemyBase* Enemy : PoolPair.Value)
        {
            if (IsValid(Enemy))
            {
                Enemy->Destroy();
            }
        }
    }
    PooledEnemies.Empty();
    
    // Destroy all active enemies
    for (auto& ActivePair : ActiveEnemies)
    {
        if (IsValid(ActivePair.Key))
        {
            ActivePair.Key->Destroy();
        }
    }
    ActiveEnemies.Empty();
    
    UE_LOG(LogEnemySpawn, Log, TEXT("Cleared enemy pool"));
}

void UEnemyPoolSubsystem::PrepoolEnemies()
{
    // This could be used to create initial pools of enemies
    // For now, we'll just initialize empty pools
    
    TArray<FName> EnemyTypes = {
        TEXT("NormalEnemy"),
        TEXT("HeavyEnemy"),
        TEXT("RangedEnemy"),
        TEXT("DashEnemy"),
        TEXT("AuraEnemy"),
        TEXT("SplitterSlime"),
        TEXT("GoldEnemy")
    };
    
    for (FName Type : EnemyTypes)
    {
        PooledEnemies.Add(Type, TArray<AEnemyBase*>());
    }
    
    UE_LOG(LogEnemySpawn, Log, TEXT("Initialized pools for %d enemy types"), EnemyTypes.Num());
}
