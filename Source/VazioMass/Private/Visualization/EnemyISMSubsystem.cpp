#include "Visualization/EnemyISMSubsystem.h"

#include "Visualization/EnemyMassVisualActor.h"

#include "Engine/World.h"

void UEnemyISMSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UEnemyISMSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        for (TWeakObjectPtr<AEnemyMassVisualActor>& Visual : VisualPool)
        {
            if (AEnemyMassVisualActor* Actor = Visual.Get())
            {
                Actor->Destroy();
            }
        }
    }

    VisualPool.Reset();
    FreeHandles.Reset();

    Super::Deinitialize();
}

bool UEnemyISMSubsystem::ShouldRenderLOD(int8 LODLevel) const
{
    // Until we have lightweight impostors for distant LODs keep everything visible.
    return LODLevel <= 3;
}

int32 UEnemyISMSubsystem::AcquireVisual(const FName ArchetypeName, const FTransform& Transform, const FVector& VisualScale)
{
    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_DedicatedServer)
    {
        return INDEX_NONE;
    }

    AEnemyMassVisualActor* VisualActor = nullptr;
    int32 Handle = INDEX_NONE;

    while (FreeHandles.Num() > 0 && Handle == INDEX_NONE)
    {
        const int32 Candidate = FreeHandles.Pop(EAllowShrinking::No);
        if (VisualPool.IsValidIndex(Candidate))
        {
            if (AEnemyMassVisualActor* CandidateActor = VisualPool[Candidate].Get())
            {
                VisualActor = CandidateActor;
                Handle = Candidate;
            }
            else
            {
                Handle = Candidate;
                VisualPool[Candidate] = nullptr;
            }
        }
    }

    if (!VisualActor)
    {
        if (Handle == INDEX_NONE)
        {
            Handle = VisualPool.Num();
            VisualPool.Add(nullptr);
        }

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        Params.ObjectFlags |= RF_Transient;

        VisualActor = World->SpawnActor<AEnemyMassVisualActor>(Params);
        if (!VisualActor)
        {
            FreeHandles.AddUnique(Handle);
            return INDEX_NONE;
        }
    }
    VisualActor->SetVisualActive(true);
    const FVector SafeScale = VisualScale.IsNearlyZero() ? FVector(1.0f) : VisualScale;
    VisualActor->ApplyTransform(Transform);
    VisualActor->ApplyArchetypeStyle(ArchetypeName, SafeScale);

    if (!VisualPool.IsValidIndex(Handle))
    {
        VisualPool.SetNum(Handle + 1);
    }
    VisualPool[Handle] = VisualActor;

    return Handle;
}

void UEnemyISMSubsystem::UpdateVisual(const int32 VisualHandle, const FTransform& Transform)
{
    if (AEnemyMassVisualActor* Visual = GetVisual(VisualHandle))
    {
        Visual->ApplyTransform(Transform);
    }
}

void UEnemyISMSubsystem::ReleaseVisual(const int32 VisualHandle)
{
    if (AEnemyMassVisualActor* Visual = GetVisual(VisualHandle))
    {
        Visual->SetVisualActive(false);
    }

    if (VisualHandle >= 0)
    {
        FreeHandles.AddUnique(VisualHandle);
    }
}

void UEnemyISMSubsystem::SetVisualActive(const int32 VisualHandle, const bool bActive)
{
    if (AEnemyMassVisualActor* Visual = GetVisual(VisualHandle))
    {
        Visual->SetVisualActive(bActive);
    }
}

AEnemyMassVisualActor* UEnemyISMSubsystem::GetVisual(const int32 VisualHandle) const
{
    if (VisualHandle < 0 || !VisualPool.IsValidIndex(VisualHandle))
    {
        return nullptr;
    }

    return VisualPool[VisualHandle].Get();
}
