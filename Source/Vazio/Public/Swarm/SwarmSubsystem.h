#pragma once
#include "Tickable.h"
#include "Subsystems/WorldSubsystem.h"
#include "Swarm/SwarmConfig.h"
#include "Swarm/Core/World.h" // ensure complete type
#include "SwarmSubsystem.generated.h"

class ASwarmManager;

UCLASS()
class VAZIO_API USwarmSubsystem : public UTickableWorldSubsystem {
	GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase&) override; 
    virtual void Deinitialize() override;
    virtual void Tick(float Dt) override; 
    virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(USwarmSubsystem, STATGROUP_Tickables); }
    virtual bool IsTickable() const override { return !bDisabled && IsInitialized() && GetWorld() && GetWorld()->IsGameWorld(); }
    virtual ETickableTickType GetTickableTickType() const override { return (!bDisabled && IsInitialized() && GetWorld() && GetWorld()->IsGameWorld()) ? ETickableTickType::Always : ETickableTickType::Never; }
	virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }
    virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override { return WorldType == EWorldType::Game || WorldType == EWorldType::PIE || WorldType == EWorldType::GamePreview; }
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    void InitWithConfig(USwarmConfig* InCfg, ASwarmManager* InMgr);
    SwarmCore::World* GetCore() const { return Core; }

    UFUNCTION(BlueprintCallable)
    void Disable();

	int32 ApplyRadialDamage(const FVector& Origin, float Radius, float DamagePerEnemy);
private:
    void DestroyCore();
    TWeakObjectPtr<ASwarmManager> Manager; 
    UPROPERTY() USwarmConfig* Cfg = nullptr; 
    SwarmCore::World* Core = nullptr;
    void StepVisuals(); 
    void BuildVisuals();
    double LastCountLogAt = 0.0;
    TArray<int32> PendingSpawnPerType;
    int32 RaycastsBudgetThisFrame = 0;
    class USwarmVisibilityComponent* Visibility = nullptr;
    bool bDisabled = false;
};
