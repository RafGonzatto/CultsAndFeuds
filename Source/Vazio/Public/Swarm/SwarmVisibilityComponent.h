#pragma once
#include "Components/ActorComponent.h"
#include "SwarmVisibilityComponent.generated.h"

namespace SwarmCore { struct World; }
class USwarmConfig;

UCLASS(ClassGroup=(Swarm), meta=(BlueprintSpawnableComponent))
class VAZIO_API USwarmVisibilityComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    USwarmVisibilityComponent();

    // Compute visible indices per type using distance + view-cone tests
    void Compute(const SwarmCore::World& W, const FVector& CamPos, const FVector& CamForward, float FOVAngle, const USwarmConfig& Cfg);

    const TArray<int32>& GetVisibleOfType(int32 TypeIdx) const { return VisibleByType.IsValidIndex(TypeIdx) ? VisibleByType[TypeIdx] : Empty; }
    int32 GetVisibleTotal() const { return VisibleTotal; }

private:
    TArray<TArray<int32>> VisibleByType;
    TArray<int32> Empty;
    int32 VisibleTotal = 0;
};
