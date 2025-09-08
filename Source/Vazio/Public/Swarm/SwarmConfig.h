#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "SwarmConfig.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogSwarm, Log, All);


UENUM(BlueprintType) enum class ESwarmShape : uint8 { Sphere, Cube, Capsule };


USTRUCT(BlueprintType)
struct FSwarmEnemyTypeConfig {
	GENERATED_BODY()

	// Core
	UPROPERTY(EditAnywhere, Category = "Core")
	float Radius = 40.f; // default smaller than player

	UPROPERTY(EditAnywhere, Category = "Core")
	float Speed = 600.f;

	UPROPERTY(EditAnywhere, Category = "Core")
	float HP = 10.f;

	UPROPERTY(EditAnywhere, Category = "Core")
	float DPS = 5.f;

	// XP entregue ao morrer (apenas para agentes de enxame, usado em drop de orbs)
	UPROPERTY(EditAnywhere, Category = "Core")
	int32 XPReward = 1;

	UPROPERTY(EditAnywhere, Category = "Core")
	int32 MaxCount = 10000;

	// Visual
	UPROPERTY(EditAnywhere, Category = "Visual")
	ESwarmShape Shape = ESwarmShape::Sphere;

	UPROPERTY(EditAnywhere, Category = "Visual")
	TSoftObjectPtr<UStaticMesh> OverrideMesh;

	UPROPERTY(EditAnywhere, Category = "Visual")
	FLinearColor Tint = FLinearColor::White;

	// Elite
	UPROPERTY(EditAnywhere, Category = "Elite")
	bool bUseSkeletalForElites = false;

	UPROPERTY(EditAnywhere, Category = "Elite")
	int32 EliteCount = 0;

	UPROPERTY(EditAnywhere, Category = "Elite")
	TSoftObjectPtr<USkeletalMesh> EliteMesh;

    // Exceptions: allow forced spawns out of view (e.g., bosses)
    UPROPERTY(EditAnywhere, Category = "Spawn")
    bool bForceSpawnOutOfView = false;
};


UCLASS(BlueprintType)
class VAZIO_API USwarmConfig : public UDataAsset {
	GENERATED_BODY()
public:
	// Core
	UPROPERTY(EditAnywhere, Category = "Core")
	int32 Seed = 1337;

	UPROPERTY(EditAnywhere, Category = "Core")
	int32 MaxAgents = 10000;

	UPROPERTY(EditAnywhere, Category = "Core")
	float CellSize = 200.f;

	UPROPERTY(EditAnywhere, Category = "Core")
	float Separation = 120.f; // keep some distance

	UPROPERTY(EditAnywhere, Category = "Core")
	float MaxSpeed = 800.f;

	UPROPERTY(EditAnywhere, Category = "Core")
	float WorldExtent = 20000.f;

	UPROPERTY(EditAnywhere, Category = "Core")
	bool bParallel = true;

	// Projectiles
	UPROPERTY(EditAnywhere, Category = "Projectiles")
	int32 ProjectilePool = 20000;

	UPROPERTY(EditAnywhere, Category = "Projectiles")
	float ProjectileSpeed = 2000.f;

	UPROPERTY(EditAnywhere, Category = "Projectiles")
	float ProjectileRadius = 20.f;

	// Visual defaults
	UPROPERTY(EditAnywhere, Category = "Visual")
	TSoftObjectPtr<UStaticMesh> DefaultSphere;

	UPROPERTY(EditAnywhere, Category = "Visual")
	TSoftObjectPtr<UStaticMesh> DefaultCube;

	UPROPERTY(EditAnywhere, Category = "Visual")
	TSoftObjectPtr<UStaticMesh> DefaultCapsule;

	// Types
	UPROPERTY(EditAnywhere, Category = "Types")
	TArray<FSwarmEnemyTypeConfig> EnemyTypes;

    // Visibility/Spawn controls (data driven)
    UPROPERTY(EditAnywhere, Category = "Visibility")
    bool FrustumCull = true;

    UPROPERTY(EditAnywhere, Category = "Visibility")
    bool DistanceCull = true;

    UPROPERTY(EditAnywhere, Category = "Visibility")
    bool OcclusionCull = false;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    bool bSpawnInViewOnly = true;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    float FOVSpawnRadius = 2500.f;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    float NearSpawn = 800.f;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    float FarSpawn = 2600.f;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    int32 MaxActive = 10000;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    int32 MaxSpawnPerFrame = 128;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    bool UseNavmeshProjection = true;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    int32 MaxRaycastPerFrame = 256;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    float MinSpawnDistanceFromPlayer = 1800.f;

    // Ground lock to avoid stacking and Z-drift
    UPROPERTY(EditAnywhere, Category = "Ground")
    bool bLockAgentsToGround = true;

    UPROPERTY(EditAnywhere, Category = "Ground")
    float GroundZHeight = 55.f; // ground level in Battle_Main

    // Logical LOD
    UPROPERTY(EditAnywhere, Category = "LOD")
    int32 LOD_Near_StepFrames = 1;

    UPROPERTY(EditAnywhere, Category = "LOD")
    int32 LOD_Medium_StepFrames = 2;

    UPROPERTY(EditAnywhere, Category = "LOD")
    int32 LOD_Far_StepFrames = 4;
};

