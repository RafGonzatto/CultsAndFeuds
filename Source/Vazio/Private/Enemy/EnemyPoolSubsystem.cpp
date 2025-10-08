#include "Enemy/EnemyPoolSubsystem.h"
#include "Enemy/EnemyBase.h"
#include "Engine/World.h"
#include "Enemy/EnemyTypes.h"
#include "Logging/VazioLogFacade.h"

void UEnemyPoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    PrepoolEnemies();

    LOG_ENEMIES(Info, TEXT("EnemyPoolSubsystem initialized"));
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
        LOG_ENEMIES(Error, TEXT("GetFromPool invalid class for %s"), *EnemyType.ToString());
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
            
            LOG_ENEMIES(Debug, TEXT("Retrieved %s from pool size=%d"),
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
        LOG_ENEMIES(Info, TEXT("Created new pooled %s"), *EnemyType.ToString());
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
        LOG_ENEMIES(Warn, TEXT("Attempted to return untracked enemy %s to pool"), *Enemy->GetName());
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
        LOG_ENEMIES(Debug, TEXT("Pool full for %s destroying enemy"), *TypeName.ToString());
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
    
    LOG_ENEMIES(Debug, TEXT("Returned %s to pool size=%d"),
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
    
    LOG_ENEMIES(Info, TEXT("Cleared enemy pool"));
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
    
    LOG_ENEMIES(Info, TEXT("Initialized pools for %d enemy types"), EnemyTypes.Num());
}
