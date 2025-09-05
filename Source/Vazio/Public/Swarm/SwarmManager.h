#pragma once
#include "GameFramework/Actor.h"
#include "Swarm/SwarmConfig.h"
#include "SwarmManager.generated.h"


class USwarmVisualComponent; class USwarmSpawnerComponent; class USwarmProjectilePool; class USwarmVisibilityComponent;
namespace SwarmCore { struct World; }


UCLASS()
class VAZIO_API ASwarmManager : public AActor {
	GENERATED_BODY()
public:
	ASwarmManager();
	UPROPERTY(EditAnywhere, Category = "Config") USwarmConfig* Config = nullptr;
	UPROPERTY(EditAnywhere, Category = "Boot") int32 AutoSpawnCount = 5000;
	UPROPERTY(EditAnywhere, Category = "Boot") bool bAutoStart = true;
    // Optional UI for exiting Battle_Main back to City
    UPROPERTY(EditAnywhere, Category = "UI") TSubclassOf<class UUserWidget> ExitWidgetClass;
	void BindCore(SwarmCore::World* W) { Core = W; }
	USwarmVisualComponent* GetVisual() const { return Visual; }
	USwarmProjectilePool* GetProjPool() const { return ProjPool; }
    USwarmSpawnerComponent* GetSpawner() const { return Spawner; }
    USwarmVisibilityComponent* GetVisibility() const { return Visibility; }
protected:
	virtual void OnConstruction(const FTransform&) override; virtual void BeginPlay() override;
private:
	UPROPERTY() USwarmVisualComponent* Visual; UPROPERTY() USwarmSpawnerComponent* Spawner; UPROPERTY() USwarmProjectilePool* ProjPool; UPROPERTY() USwarmVisibilityComponent* Visibility; SwarmCore::World* Core = nullptr;
    UPROPERTY() UUserWidget* ExitWidgetInstance = nullptr;
};

