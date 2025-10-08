#include "Systems/EnemyMassSpawnerSubsystem.h"

#include "Enemy/EnemyConfig.h"
#include "Fragments/EnemyAgentFragment.h"
#include "Fragments/EnemyCombatFragment.h"
#include "Fragments/EnemyConfigSharedFragment.h"
#include "Fragments/EnemyTargetFragment.h"
#include "MassCommonFragments.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "Kismet/GameplayStatics.h"
#include "Visualization/EnemyLODFragment.h"
#include "Visualization/EnemyISMSubsystem.h"
#include "Fragments/EnemyVisualFragment.h"
#include "Containers/Ticker.h"
#include "Logging/VazioLogFacade.h"

DEFINE_LOG_CATEGORY_STATIC(LogEnemyMassSpawn, Log, All);

void UEnemyMassSpawnerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UWorld* World = GetWorld())
	{
		EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	}
}

void UEnemyMassSpawnerSubsystem::SpawnEnemies(FName ArchetypeName, int32 Count, const FVector& SpawnOrigin, const FEnemyInstanceModifiers& Modifiers)
{
	if (!EntitySubsystem.IsValid())
	{
		LOG_MASS(Warn, TEXT("Mass spawner missing MassEntitySubsystem."));
		return;
	}

	UEnemyConfig* Config = CachedConfig.Get();
	if (!Config)
	{
		LOG_MASS(Warn, TEXT("Mass spawner requires an active EnemyConfig."));
		return;
	}

	const FEnemyArchetype* ArchetypeData = Config->GetArchetype(ArchetypeName);
	if (!ArchetypeData)
	{
		LOG_MASS(Warn, TEXT("No archetype data for %s."), *ArchetypeName.ToString());
		return;
	}

	const FArchetypeRecord Record = BuildOrGetArchetype(ArchetypeName, *ArchetypeData, Modifiers);
	if (Record.ArchetypeID == INDEX_NONE)
	{
		LOG_MASS(Warn, TEXT("Failed to build archetype for %s."), *ArchetypeName.ToString());
		return;
	}

	LOG_MASS(Info, TEXT("[MassStub] Would spawn %d of %s at around %s"), Count, *ArchetypeName.ToString(), *SpawnOrigin.ToString());
}

void UEnemyMassSpawnerSubsystem::SpawnEnemiesAtTransforms(FName ArchetypeName, const TArray<FTransform>& SpawnTransforms, const FEnemyInstanceModifiers& Modifiers)
{
	if (SpawnTransforms.IsEmpty())
	{
		return;
	}

	if (!EntitySubsystem.IsValid())
	{
		LOG_MASS(Warn, TEXT("Mass spawner missing subsystem references for %s"), *ArchetypeName.ToString());
		return;
	}

	UEnemyConfig* Config = CachedConfig.Get();
	if (!Config)
	{
		LOG_MASS(Warn, TEXT("Mass spawner requires an active EnemyConfig for %s"), *ArchetypeName.ToString());
		return;
	}

	const FEnemyArchetype* ArchetypeData = Config->GetArchetype(ArchetypeName);
	if (!ArchetypeData)
	{
		LOG_MASS(Warn, TEXT("No archetype data for %s"), *ArchetypeName.ToString());
		return;
	}

	const FArchetypeRecord Record = BuildOrGetArchetype(ArchetypeName, *ArchetypeData, Modifiers);
	if (Record.ArchetypeID == INDEX_NONE)
	{
		LOG_MASS(Warn, TEXT("Failed to build archetype for %s"), *ArchetypeName.ToString());
		return;
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	TArray<FMassEntityHandle> NewEntities;
	NewEntities.Reserve(SpawnTransforms.Num());

	// Determine initial target as player location
	FVector TargetLocation = FVector::ZeroVector;
	if (UWorld* World = GetWorld())
	{
		if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
		{
			TargetLocation = PlayerPawn->GetActorLocation();
		}
	}

	// Create entities
	for (int32 i = 0; i < SpawnTransforms.Num(); ++i)
	{
		const FMassEntityHandle Entity = EntityManager.CreateEntity(Record.ArchetypeHandle);
		NewEntities.Add(Entity);
	}

	UEnemyISMSubsystem* VisualSubsystem = nullptr;
	if (UWorld* World = GetWorld())
	{
		VisualSubsystem = World->GetSubsystem<UEnemyISMSubsystem>();
	}

	// Initialize fragments
	for (int32 i = 0; i < NewEntities.Num(); ++i)
	{
		const FMassEntityHandle Entity = NewEntities[i];
		const FTransform& Xf = SpawnTransforms[i];
		ConfigureFragments(Entity, ArchetypeName, *ArchetypeData, Modifiers, Xf.GetLocation(), TargetLocation, Xf.GetRotation());

		FEnemyAgentFragment* Agent = EntityManager.GetFragmentDataPtr<FEnemyAgentFragment>(Entity);
		if (Agent)
		{
			Agent->ArchetypeID = Record.ArchetypeID;
			Agent->Health = ArchetypeData->BaseHP * FMath::Max(Modifiers.HealthMultiplier, 0.01f);
		}

		if (FEnemyConfigSharedFragment* ConfigFragment = EntityManager.GetFragmentDataPtr<FEnemyConfigSharedFragment>(Entity))
		{
			ConfigFragment->Health = ArchetypeData->BaseHP * FMath::Max(Modifiers.HealthMultiplier, 0.01f);
			ConfigFragment->Speed = ArchetypeData->BaseSpeed * FMath::Max(Modifiers.SpeedMultiplier, 0.01f);
			ConfigFragment->Damage = ArchetypeData->BaseDMG * FMath::Max(Modifiers.DamageMultiplier, 0.01f);
			ConfigFragment->XPReward = FMath::RoundToInt(ArchetypeData->BaseRewards.XPValue * Modifiers.RewardMultiplier);
		}

		// Assign NetID on server
		if (UWorld* World = GetWorld())
		{
			const ENetMode NetMode = World->GetNetMode();
			if ((NetMode == NM_DedicatedServer || NetMode == NM_ListenServer) && Agent)
			{
				Agent->NetID = NextNetID++;
			}
		}

		if (VisualSubsystem)
		{
			const FTransform VisualTransform = Agent ? Agent->CachedTransform : Xf;
			const float BaseScale = ArchetypeData->BaseSize * FMath::Max(Modifiers.ScaleMultiplier, 0.1f);
			const FVector VisualScale(BaseScale);

			if (FEnemyVisualHandleFragment* VisualHandle = EntityManager.GetFragmentDataPtr<FEnemyVisualHandleFragment>(Entity))
			{
				VisualHandle->ArchetypeName = ArchetypeName;
				VisualHandle->VisualHandle = VisualSubsystem->AcquireVisual(ArchetypeName, VisualTransform, VisualScale);
			}
		}
		else if (FEnemyVisualHandleFragment* VisualHandle = EntityManager.GetFragmentDataPtr<FEnemyVisualHandleFragment>(Entity))
		{
			VisualHandle->VisualHandle = INDEX_NONE;
			VisualHandle->ArchetypeName = ArchetypeName;
		}
	}

	LOG_MASS(Info, TEXT("[Mass] Spawned %d of %s (first=%s)"), NewEntities.Num(), *ArchetypeName.ToString(), *SpawnTransforms[0].GetLocation().ToString());
}

FBossMassHandle UEnemyMassSpawnerSubsystem::SpawnBossAtTransforms(FName BossType, const TArray<FTransform>& SpawnTransforms, const FEnemyInstanceModifiers& Modifiers)
{
	FBossMassHandle Handle;

	if (SpawnTransforms.IsEmpty())
	{
		LOG_MASS(Warn, TEXT("SpawnBossAtTransforms requires at least one transform for %s"), *BossType.ToString());
		return Handle;
	}

	if (!EntitySubsystem.IsValid())
	{
		LOG_MASS(Warn, TEXT("Mass spawner missing subsystem references for boss %s"), *BossType.ToString());
		return Handle;
	}

	UEnemyConfig* Config = CachedConfig.Get();
	if (!Config)
	{
		LOG_MASS(Warn, TEXT("Mass spawner requires an active EnemyConfig for boss %s"), *BossType.ToString());
		return Handle;
	}

	const FEnemyArchetype* ArchetypeData = Config->GetArchetype(BossType);
	if (!ArchetypeData)
	{
		LOG_MASS(Warn, TEXT("No archetype data for boss %s"), *BossType.ToString());
		return Handle;
	}

	const FArchetypeRecord Record = BuildOrGetArchetype(BossType, *ArchetypeData, Modifiers);
	if (Record.ArchetypeID == INDEX_NONE)
	{
		LOG_MASS(Warn, TEXT("Failed to build archetype for boss %s"), *BossType.ToString());
		return Handle;
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	const FMassEntityHandle Entity = EntityManager.CreateEntity(Record.ArchetypeHandle);

	const FTransform& BossTransform = SpawnTransforms[0];

	// Determine current player location for targeting.
	FVector TargetLocation = BossTransform.GetLocation();
	if (UWorld* World = GetWorld())
	{
		if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
		{
			TargetLocation = PlayerPawn->GetActorLocation();
		}
	}

	ConfigureFragments(Entity, BossType, *ArchetypeData, Modifiers, BossTransform.GetLocation(), TargetLocation, BossTransform.GetRotation());

	const float MaxHealth = ArchetypeData->BaseHP * FMath::Max(Modifiers.HealthMultiplier, 0.01f);

	FEnemyAgentFragment* Agent = EntityManager.GetFragmentDataPtr<FEnemyAgentFragment>(Entity);
	if (Agent)
	{
		Agent->ArchetypeID = Record.ArchetypeID;
		Agent->Health = MaxHealth;
	}

	if (FEnemyConfigSharedFragment* ConfigFragment = EntityManager.GetFragmentDataPtr<FEnemyConfigSharedFragment>(Entity))
	{
		ConfigFragment->Health = MaxHealth;
		ConfigFragment->Speed = ArchetypeData->BaseSpeed * FMath::Max(Modifiers.SpeedMultiplier, 0.01f);
		ConfigFragment->Damage = ArchetypeData->BaseDMG * FMath::Max(Modifiers.DamageMultiplier, 0.01f);
		ConfigFragment->XPReward = FMath::RoundToInt(ArchetypeData->BaseRewards.XPValue * Modifiers.RewardMultiplier);
	}

	Handle.Entity = Entity;
	Handle.BossType = BossType;
	Handle.MaxHealth = MaxHealth;

	FBossMassInstance Instance;
	Instance.Handle = Handle;
	Instance.SpawnTransform = BossTransform;
	Instance.LastNormalizedHealth = 1.f;

	ActiveBosses.Add(Instance);
	StartBossTracking();

	OnBossSpawned.Broadcast(Handle, BossTransform);

	if (UWorld* World = GetWorld())
	{
		if (UEnemyISMSubsystem* VisualSubsystem = World->GetSubsystem<UEnemyISMSubsystem>())
		{
			if (FEnemyVisualHandleFragment* VisualHandle = EntityManager.GetFragmentDataPtr<FEnemyVisualHandleFragment>(Entity))
			{
				const float BaseScale = ArchetypeData->BaseSize * FMath::Max(Modifiers.ScaleMultiplier, 0.1f);
				const FVector VisualScale(BaseScale);
				VisualHandle->ArchetypeName = BossType;
				VisualHandle->VisualHandle = VisualSubsystem->AcquireVisual(BossType, Agent ? Agent->CachedTransform : BossTransform, VisualScale);
			}
		}
		else if (FEnemyVisualHandleFragment* VisualHandle = EntityManager.GetFragmentDataPtr<FEnemyVisualHandleFragment>(Entity))
		{
			VisualHandle->VisualHandle = INDEX_NONE;
			VisualHandle->ArchetypeName = BossType;
		}
	}

	LOG_MASS(Info, TEXT("[Mass] Spawned boss %s at %s"), *BossType.ToString(), *BossTransform.GetLocation().ToString());

	return Handle;
}

bool UEnemyMassSpawnerSubsystem::TryGetBossHealth(const FBossMassHandle& BossHandle, float& OutCurrentHealth, float& OutMaxHealth)
{
	OutCurrentHealth = 0.f;
	OutMaxHealth = 0.f;

	if (!BossHandle.IsValid() || !EntitySubsystem.IsValid())
	{
		return false;
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	if (!EntityManager.IsEntityValid(BossHandle.Entity))
	{
		return false;
	}

	if (FEnemyAgentFragment* Agent = EntityManager.GetFragmentDataPtr<FEnemyAgentFragment>(BossHandle.Entity))
	{
		OutCurrentHealth = Agent->Health;
	}
	else
	{
		return false;
	}

	if (FEnemyConfigSharedFragment* ConfigFragment = EntityManager.GetFragmentDataPtr<FEnemyConfigSharedFragment>(BossHandle.Entity))
	{
		OutMaxHealth = ConfigFragment->Health;
	}

	if (OutMaxHealth <= 0.f)
	{
		OutMaxHealth = BossHandle.MaxHealth;
	}

	return true;
}

UEnemyMassSpawnerSubsystem::FArchetypeRecord UEnemyMassSpawnerSubsystem::BuildOrGetArchetype(FName ArchetypeName, const FEnemyArchetype& ArchetypeData, const FEnemyInstanceModifiers& Modifiers)
{
	if (FArchetypeRecord* ExistingRecord = ArchetypeCache.Find(ArchetypeName))
	{
		return *ExistingRecord;
	}

	if (!EntitySubsystem.IsValid())
	{
		return FArchetypeRecord();
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();

	FArchetypeRecord Record;
	Record.ArchetypeID = NextArchetypeID++;

	TArray<const UScriptStruct*> FragmentTypes;
	FragmentTypes.Reserve(7);
	FragmentTypes.Add(FEnemyAgentFragment::StaticStruct());
	FragmentTypes.Add(FEnemyCombatFragment::StaticStruct());
	FragmentTypes.Add(FEnemyTargetFragment::StaticStruct());
	FragmentTypes.Add(FEnemyConfigSharedFragment::StaticStruct());
	FragmentTypes.Add(FTransformFragment::StaticStruct());
	FragmentTypes.Add(FEnemyLODLevelFragment::StaticStruct());
	FragmentTypes.Add(FEnemyVisualHandleFragment::StaticStruct());

	const FMassArchetypeHandle Archetype = EntityManager.CreateArchetype(FragmentTypes);

	Record.ArchetypeHandle = Archetype;

	ArchetypeCache.Add(ArchetypeName, Record);
	ArchetypeReverseLookup.Add(Record.ArchetypeID, ArchetypeName);
	return Record;
}

FName UEnemyMassSpawnerSubsystem::GetArchetypeName(int32 ArchetypeID) const
{
	if (const FName* FoundName = ArchetypeReverseLookup.Find(ArchetypeID))
	{
		return *FoundName;
	}

	return NAME_None;
}

FName UEnemyMassSpawnerSubsystem::ResolveArchetypeName(int32 ArchetypeID) const
{
	return GetArchetypeName(ArchetypeID);
}

void UEnemyMassSpawnerSubsystem::ConfigureFragments(const FMassEntityHandle EntityHandle, FName ArchetypeName, const FEnemyArchetype& ArchetypeData, const FEnemyInstanceModifiers& Modifiers, const FVector& SpawnLocation, const FVector& TargetLocation, const FQuat& OptionalRotation)
{
	if (!EntitySubsystem.IsValid())
	{
		return;
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();

	if (!EntityManager.IsEntityValid(EntityHandle))
	{
		return;
	}

	FTransform SpawnXf(OptionalRotation, SpawnLocation);

	if (FTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FTransformFragment>(EntityHandle))
	{
		Transform->SetTransform(SpawnXf);
	}

	if (FEnemyAgentFragment* Agent = EntityManager.GetFragmentDataPtr<FEnemyAgentFragment>(EntityHandle))
	{
		Agent->Position = SpawnLocation;
		Agent->Velocity = FVector::ZeroVector;
		Agent->DesiredSpeed = ArchetypeData.BaseSpeed * FMath::Max(Modifiers.SpeedMultiplier, 0.01f);
		Agent->Health = ArchetypeData.BaseHP * FMath::Max(Modifiers.HealthMultiplier, 0.01f);
		Agent->CachedTransform = SpawnXf;
	}

	if (FEnemyTargetFragment* Target = EntityManager.GetFragmentDataPtr<FEnemyTargetFragment>(EntityHandle))
	{
		Target->TargetLocation = TargetLocation;
		Target->DesiredDirection = (TargetLocation - SpawnLocation).GetSafeNormal();
		Target->DesiredSpeedScale = 1.0f;
	}

	if (FEnemyCombatFragment* Combat = EntityManager.GetFragmentDataPtr<FEnemyCombatFragment>(EntityHandle))
	{
		Combat->AttackRange = 180.0f;
		Combat->PreferredRange = 520.0f;
		Combat->CohesionWeight = 0.45f;
		Combat->AlignmentWeight = 0.35f;
		Combat->GroupRadius = 600.0f * FMath::Max(Modifiers.ScaleMultiplier, 0.5f);
		Combat->DashCooldown = ArchetypeData.DashCooldown;
		Combat->DashSpeedMultiplier = 2.4f;
		Combat->AbilityFlags = EEnemyAbilityFlags::None;
		if (ArchetypeData.bCanDash)
		{
			Combat->AbilityFlags = (EEnemyAbilityFlags)((uint8)Combat->AbilityFlags | (uint8)EEnemyAbilityFlags::Dash);
		}
		if (ArchetypeData.bHasAura)
		{
			Combat->AbilityFlags = (EEnemyAbilityFlags)((uint8)Combat->AbilityFlags | (uint8)EEnemyAbilityFlags::Aura);
		}
	}

	if (FEnemyConfigSharedFragment* Config = EntityManager.GetFragmentDataPtr<FEnemyConfigSharedFragment>(EntityHandle))
	{
		Config->Health = ArchetypeData.BaseHP * FMath::Max(Modifiers.HealthMultiplier, 0.01f);
		Config->Speed = ArchetypeData.BaseSpeed * FMath::Max(Modifiers.SpeedMultiplier, 0.01f);
		Config->Damage = ArchetypeData.BaseDMG * FMath::Max(Modifiers.DamageMultiplier, 0.01f);
		Config->XPReward = FMath::RoundToInt(ArchetypeData.BaseRewards.XPValue * Modifiers.RewardMultiplier);
	}
}

void UEnemyMassSpawnerSubsystem::SetEnemyConfig(UEnemyConfig* Config)
{
	CachedConfig = Config;
	ArchetypeCache.Reset();
	ArchetypeReverseLookup.Reset();
	NextArchetypeID = 1;
}

void UEnemyMassSpawnerSubsystem::StartBossTracking()
{
	if (!BossTickerHandle.IsValid())
	{
		BossTickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UEnemyMassSpawnerSubsystem::TickBossTracking));
	}
}

void UEnemyMassSpawnerSubsystem::StopBossTracking()
{
	if (BossTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(BossTickerHandle);
		BossTickerHandle = FTSTicker::FDelegateHandle();
	}
}

bool UEnemyMassSpawnerSubsystem::TickBossTracking(float DeltaTime)
{
	if (!EntitySubsystem.IsValid() || ActiveBosses.Num() == 0)
	{
		StopBossTracking();
		return false;
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();

	for (int32 Index = ActiveBosses.Num() - 1; Index >= 0; --Index)
	{
		FBossMassInstance& Instance = ActiveBosses[Index];

		if (!EntityManager.IsEntityValid(Instance.Handle.Entity))
		{
			OnBossDefeated.Broadcast(Instance.Handle);
			ActiveBosses.RemoveAtSwap(Index);
			continue;
		}

		float CurrentHealth = 0.f;
		float MaxHealth = 0.f;
		if (!TryGetBossHealth(Instance.Handle, CurrentHealth, MaxHealth))
		{
			ActiveBosses.RemoveAtSwap(Index);
			continue;
		}

		const float Normalized = MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f;

		if (!FMath::IsNearlyEqual(Normalized, Instance.LastNormalizedHealth, 0.001f))
		{
			Instance.LastNormalizedHealth = Normalized;
			OnBossHealthChanged.Broadcast(Instance.Handle, Normalized);
		}

		if (CurrentHealth <= 0.f)
		{
			OnBossDefeated.Broadcast(Instance.Handle);
			EntityManager.DestroyEntity(Instance.Handle.Entity);
			ActiveBosses.RemoveAtSwap(Index);
		}
	}

	if (ActiveBosses.Num() == 0)
	{
		StopBossTracking();
		return false;
	}

	return true;
}
    
