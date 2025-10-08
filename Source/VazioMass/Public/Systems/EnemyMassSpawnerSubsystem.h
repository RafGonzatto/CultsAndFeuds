#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Enemy/EnemyTypes.h"
#include "MassEntityTypes.h"
#include "Fragments/EnemyAgentFragment.h"
#include "Fragments/EnemyCombatFragment.h"
#include "Fragments/EnemyTargetFragment.h"
#include "Fragments/EnemyConfigSharedFragment.h"
#include "MassCommonFragments.h"
#include "MassEntityManager.h"
#include "Containers/Ticker.h"

#include "EnemyMassSpawnerSubsystem.generated.h"

struct FMassEntityHandle;
struct FEnemyArchetype;
struct FEnemyInstanceModifiers;

class UMassEntitySubsystem;
class UEnemyConfig;
struct FMassCommandBuffer;

struct FBossMassHandle
{
    FMassEntityHandle Entity;
    FName BossType = NAME_None;
    float MaxHealth = 0.f;

    bool IsValid() const
    {
        return Entity.IsSet() && !BossType.IsNone();
    }
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMassBossSpawned, const FBossMassHandle&, const FTransform& /*SpawnTransform*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMassBossHealthChanged, const FBossMassHandle&, float /*NormalizedHealth*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMassBossDefeated, const FBossMassHandle&);

UCLASS()
class VAZIOMASS_API UEnemyMassSpawnerSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    /** Spawns a batch of mass-based enemies using the legacy archetype data. */
    void SpawnEnemies(FName ArchetypeName, int32 Count, const FVector& SpawnOrigin, const FEnemyInstanceModifiers& Modifiers);

    /** Spawns enemies at explicit transforms, preserving authored placement where needed. */
    void SpawnEnemiesAtTransforms(FName ArchetypeName, const TArray<FTransform>& SpawnTransforms, const FEnemyInstanceModifiers& Modifiers);

    /**
     * Spawns a boss enemy via the Mass pipeline and registers it for lifetime tracking.
     * @param BossType The type tag that identifies which boss archetype to instantiate.
     * @param SpawnTransforms One or more transforms used to position the spawned boss. Only the first entry is consumed today.
     * @param Modifiers Gameplay modifiers (health, damage, etc.) applied to the boss fragments at spawn time.
     * @return Handle describing the spawned boss entity; invalid when spawning fails.
     */
    FBossMassHandle SpawnBossAtTransforms(FName BossType, const TArray<FTransform>& SpawnTransforms, const FEnemyInstanceModifiers& Modifiers);

    /** Queries boss health using the Mass fragments. Returns false if the entity is invalid. */
    bool TryGetBossHealth(const FBossMassHandle& BossHandle, float& OutCurrentHealth, float& OutMaxHealth);

    /** Public helper for replication/client: resolve archetype display name from ID. */
    FName ResolveArchetypeName(int32 ArchetypeID) const;

    /** Injects the enemy configuration asset used to resolve archetypes. */
    void SetEnemyConfig(UEnemyConfig* Config);

    FOnMassBossSpawned OnBossSpawned;
    FOnMassBossHealthChanged OnBossHealthChanged;
    FOnMassBossDefeated OnBossDefeated;

private:
    struct FArchetypeRecord
    {
        int32 ArchetypeID = INDEX_NONE;
        /** Cached Mass archetype used to create entities for this enemy type. */
        FMassArchetypeHandle ArchetypeHandle;
    };

    FArchetypeRecord BuildOrGetArchetype(FName ArchetypeName, const FEnemyArchetype& ArchetypeData, const FEnemyInstanceModifiers& Modifiers);
    FName GetArchetypeName(int32 ArchetypeID) const;

    void ConfigureFragments(const FMassEntityHandle EntityHandle, FName ArchetypeName, const FEnemyArchetype& ArchetypeData, const FEnemyInstanceModifiers& Modifiers, const FVector& SpawnLocation, const FVector& TargetLocation, const FQuat& OptionalRotation = FQuat::Identity);

    void StartBossTracking();
    void StopBossTracking();
    bool TickBossTracking(float DeltaTime);

private:
    TWeakObjectPtr<UMassEntitySubsystem> EntitySubsystem;
    TWeakObjectPtr<UEnemyConfig> CachedConfig;

    TMap<FName, FArchetypeRecord> ArchetypeCache;
    TMap<int32, FName> ArchetypeReverseLookup;
    int32 NextArchetypeID = 1;
    int32 NextNetID = 1;

    struct FBossMassInstance
    {
        FBossMassHandle Handle;
        FTransform SpawnTransform;
        float LastNormalizedHealth = 1.f;
    };

    TArray<FBossMassInstance> ActiveBosses;
    FTSTicker::FDelegateHandle BossTickerHandle;
};
