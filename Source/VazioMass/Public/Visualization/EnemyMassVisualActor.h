#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyMassVisualActor.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/** Simple visual proxy spawned for Mass-driven enemies. */
UCLASS()
class VAZIOMASS_API AEnemyMassVisualActor : public AActor
{
    GENERATED_BODY()

public:
    AEnemyMassVisualActor();

    /** Applies archetype-specific styling and scale to the proxy. */
    void ApplyArchetypeStyle(FName ArchetypeName, const FVector& VisualScale);

    /** Updates the actor transform without triggering physics. */
    void ApplyTransform(const FTransform& InTransform);

    /** Toggles visibility without affecting pooling state. */
    void SetVisualActive(bool bActive);

protected:
    virtual void BeginPlay() override;

private:
    void EnsureMaterial();
    void ApplyColor(const FLinearColor& Color);

private:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UStaticMeshComponent> VisualComponent;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

    FName CachedArchetype;
};
