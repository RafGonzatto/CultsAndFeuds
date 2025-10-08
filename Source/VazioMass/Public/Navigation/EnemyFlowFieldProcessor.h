#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassCommonTypes.h"
#include "MassEntityManager.h"
#include "EnemyFlowFieldProcessor.generated.h"

class UEnemyPerceptionService;

/**
 * Generates and applies flow-field navigation data to smooth large scale crowd movement.
 */
UCLASS()
class VAZIOMASS_API UEnemyFlowFieldProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UEnemyFlowFieldProcessor();

protected:
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    void RequestFlowField(const FVector& Center);
    void ApplyFlowField(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

private:
    FMassEntityQuery EntityQuery;

    /** World-space center used for flow-field generation (typically the player's location). */
    FVector LastFlowFieldCenter = FVector::ZeroVector;
    float TimeSinceLastUpdate = 0.0f;

    float GridSize = 10000.0f;
    float CellSize = 300.0f;
    float FlowFieldUpdateInterval = 0.2f;
    float FlowFieldUpdateDistance = 600.0f;
    float DirectPathThreshold = 900.0f;

    bool bFlowFieldDirty = true;

    UPROPERTY()
    TObjectPtr<UEnemyPerceptionService> PerceptionService;

public:
    struct FFlowField
    {
        FVector Center;
        float CellSize = 300.0f;
        int32 HalfExtent = 16;
        TArray<FVector> Directions;

        FVector GetFlowAtPosition(const FVector& Position) const
        {
            if (Directions.Num() == 0)
            {
                return FVector::ZeroVector;
            }

            const FVector Offset = Position - Center;
            const int32 CellX = FMath::Clamp(FMath::FloorToInt(Offset.X / CellSize), -HalfExtent, HalfExtent - 1);
            const int32 CellY = FMath::Clamp(FMath::FloorToInt(Offset.Y / CellSize), -HalfExtent, HalfExtent - 1);

            const int32 LocalX = CellX + HalfExtent;
            const int32 LocalY = CellY + HalfExtent;
            const int32 Index = LocalY * (HalfExtent * 2) + LocalX;
            if (!Directions.IsValidIndex(Index))
            {
                return FVector::ZeroVector;
            }

            return Directions[Index];
        }
    };
    TSharedPtr<FFlowField> ActiveFlowField;
};
