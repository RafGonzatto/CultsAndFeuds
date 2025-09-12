#pragma once

#include "CoreMinimal.h"
#include "Enemy/EnemyBase.h"
#include "SplitterSlime.generated.h"

UCLASS()
class VAZIO_API ASplitterSlime : public AEnemyBase
{
    GENERATED_BODY()

public:
    ASplitterSlime();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void HandleDeath(bool bIsParentParam = false) override;

private:
    void PursuePlayer(float DeltaTime);
    void CreateChildren();
    
    UPROPERTY(EditAnywhere, Category = "AI")
    float PursuitSpeed = 200.f;
    
    UPROPERTY(EditAnywhere, Category = "AI")
    float StoppingDistance = 100.f;
    
    UPROPERTY(EditAnywhere, Category = "Splitting")
    int32 ChildrenCount = 2;
    
    UPROPERTY(EditAnywhere, Category = "Splitting")
    float ChildrenSpawnRadius = 100.f;
    
    UPROPERTY(EditAnywhere, Category = "Splitting")
    float ChildrenHPMultiplier = 0.5f;
    
    UPROPERTY(EditAnywhere, Category = "Splitting")
    float ChildrenDMGMultiplier = 0.5f;
};
