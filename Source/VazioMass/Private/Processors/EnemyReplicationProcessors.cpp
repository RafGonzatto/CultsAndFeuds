#include "Processors/EnemyReplicationProcessors.h"

#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassEntityManager.h"
#include "MassCommonFragments.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "EngineUtils.h"

#include "Networking/EnemyMassReplicator.h"
#include "Fragments/EnemyAgentFragment.h"
#include "Fragments/EnemyTargetFragment.h"
#include "Fragments/EnemyCombatFragment.h"
#include "Fragments/EnemyConfigSharedFragment.h"
#include "Systems/EnemyMassSpawnerSubsystem.h"

// --- Server Processor ---

UEnemyReplicationServerProcessor::UEnemyReplicationServerProcessor()
{
    EntityQuery.RegisterWithProcessor(*this);
    ExecutionOrder.ExecuteInGroup = FName(TEXT("PostPhysics"));
    bRequiresGameThreadExecution = true; // interacting with replicated actor
}

void UEnemyReplicationServerProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    (void)EntityManager;
    EntityQuery.AddRequirement<FEnemyAgentFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FEnemyConfigSharedFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
}

void UEnemyReplicationServerProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    UWorld* World = Context.GetWorld();
    if (!World)
    {
        return;
    }
    const ENetMode NetMode = World->GetNetMode();
    if (!(NetMode == NM_DedicatedServer || NetMode == NM_ListenServer))
    {
        return;
    }

    AEnemyMassReplicator* Replicator = nullptr;
    for (TActorIterator<AEnemyMassReplicator> It(World); It; ++It)
    {
        Replicator = *It;
        break;
    }
    if (!Replicator)
    {
        FActorSpawnParameters P; P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        Replicator = World->SpawnActor<AEnemyMassReplicator>(AEnemyMassReplicator::StaticClass(), FTransform::Identity, P);
        if (!Replicator) return;
    }

    TArray<FEnemyMassNetDatum> Snapshot;
    Snapshot.Reserve(1024);

    EntityQuery.ForEachEntityChunk(Context, [&Snapshot](FMassExecutionContext& ChunkContext)
    {
        const TConstArrayView<FEnemyAgentFragment> Agents = ChunkContext.GetFragmentView<FEnemyAgentFragment>();
        const TConstArrayView<FEnemyConfigSharedFragment> Configs = ChunkContext.GetFragmentView<FEnemyConfigSharedFragment>();
        const TConstArrayView<FTransformFragment> Transforms = ChunkContext.GetFragmentView<FTransformFragment>();

        for (int32 i = 0; i < ChunkContext.GetNumEntities(); ++i)
        {
            FEnemyMassNetDatum D;
            D.NetID = Agents[i].NetID;
            D.ArchetypeID = Agents[i].ArchetypeID;
            D.Location = Transforms[i].GetTransform().GetLocation();
            D.Rotation = Transforms[i].GetTransform().GetRotation().Rotator();
            D.Health = Agents[i].Health;
            Snapshot.Add(D);
        }
    });

    Replicator->NetStates = MoveTemp(Snapshot);
    Replicator->ForceNetUpdate();
}

// --- Client Processor ---

UEnemyReplicationClientProcessor::UEnemyReplicationClientProcessor()
{
    EntityQuery.RegisterWithProcessor(*this);
    ExecutionOrder.ExecuteInGroup = FName(TEXT("PrePhysics"));
    bRequiresGameThreadExecution = true; // may interact with spawner and world
}

void UEnemyReplicationClientProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    (void)EntityManager;
    EntityQuery.AddRequirement<FEnemyAgentFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FEnemyConfigSharedFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
}

void UEnemyReplicationClientProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    UWorld* World = Context.GetWorld();
    if (!World || World->GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    AEnemyMassReplicator* Replicator = nullptr;
    for (TActorIterator<AEnemyMassReplicator> It(World); It; ++It)
    {
        Replicator = *It;
        break;
    }
    if (!Replicator || Replicator->NetStates.Num() == 0)
    {
        return;
    }

    UEnemyMassSpawnerSubsystem* Spawner = World->GetSubsystem<UEnemyMassSpawnerSubsystem>();
    if (!Spawner)
    {
        return;
    }

    // Build a quick NetID -> Entity map
    TMap<int32, FMassEntityHandle> NetMap;
    EntityQuery.ForEachEntityChunk(Context, [&NetMap, &EntityManager](FMassExecutionContext& ChunkContext)
    {
        const TConstArrayView<FEnemyAgentFragment> Agents = ChunkContext.GetFragmentView<FEnemyAgentFragment>();
        const TConstArrayView<FMassEntityHandle> Entities = ChunkContext.GetEntities();
        for (int32 i = 0; i < ChunkContext.GetNumEntities(); ++i)
        {
            if (Agents[i].NetID != INDEX_NONE)
            {
                NetMap.Add(Agents[i].NetID, Entities[i]);
            }
        }
    });

    // Apply snapshot: spawn missing ones, update transforms/health
    for (const FEnemyMassNetDatum& D : Replicator->NetStates)
    {
        if (D.NetID == INDEX_NONE)
        {
            continue;
        }

        FMassEntityHandle* Found = NetMap.Find(D.NetID);
        if (!Found)
        {
            // Spawn a placeholder mass enemy for this NetID using archetype.
            const FName ArchetypeName = Spawner->ResolveArchetypeName(D.ArchetypeID);
            if (ArchetypeName.IsNone())
            {
                continue;
            }
            TArray<FTransform> T;
            T.Add(FTransform(D.Rotation, D.Location));
            FEnemyInstanceModifiers Mods; // server already baked values into replication, modifiers mostly irrelevant on client
            Spawner->SpawnEnemiesAtTransforms(ArchetypeName, T, Mods);
            // Next frame it will get mapped and updated.
            continue;
        }

        FMassEntityHandle Entity = *Found;
        if (FTransformFragment* Tr = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity))
        {
            Tr->SetTransform(FTransform(D.Rotation, FVector(D.Location)));
        }
        if (FEnemyAgentFragment* Agent = EntityManager.GetFragmentDataPtr<FEnemyAgentFragment>(Entity))
        {
            Agent->Health = D.Health;
        }
    }
}
