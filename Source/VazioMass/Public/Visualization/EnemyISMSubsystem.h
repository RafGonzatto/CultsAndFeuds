#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnemyISMSubsystem.generated.h"

class AEnemyMassVisualActor;

/** Manages pooled visual proxies for Mass-driven enemies and applies LOD policies. */
UCLASS()
class VAZIOMASS_API UEnemyISMSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Returns whether the provided LOD level should be visible. */
    bool ShouldRenderLOD(int8 LODLevel) const;

    /** Allocates or reuses a visual proxy for the given archetype. */
    int32 AcquireVisual(FName ArchetypeName, const FTransform& Transform, const FVector& VisualScale);

    /** Updates the world transform for the specified visual handle. */
    void UpdateVisual(int32 VisualHandle, const FTransform& Transform);

    /** Releases a previously acquired visual instance back into the pool. */
    void ReleaseVisual(int32 VisualHandle);

    /** Toggles visibility for the provided handle according to LOD. */
    void SetVisualActive(int32 VisualHandle, bool bActive);

private:
    AEnemyMassVisualActor* GetVisual(int32 VisualHandle) const;

private:
    TArray<TWeakObjectPtr<AEnemyMassVisualActor>> VisualPool;
    TArray<int32> FreeHandles;
};
