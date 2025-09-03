#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WorldPlacerConfig.generated.h"

USTRUCT(BlueprintType)
struct FWorldPlaceEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Placement")
	TObjectPtr<UStaticMesh> Mesh = nullptr;

	UPROPERTY(EditAnywhere, Category="Placement")
	FTransform Transform = FTransform::Identity;

	UPROPERTY(EditAnywhere, Category="Placement")
	bool bMovable = false;

	UPROPERTY(EditAnywhere, Category="Placement")
	FName CollisionProfile = "BlockAll";

	UPROPERTY(EditAnywhere, Category="Placement")
	bool bUseHISM = true;

	UPROPERTY(EditAnywhere, Category="Placement")
	FName Tag;

	UPROPERTY(EditAnywhere, Category="Placement")
	bool bInteractable = false;
};

UCLASS(BlueprintType)
class VAZIO_API UWorldPlacerConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Placement")
	TArray<FWorldPlaceEntry> Entries;
};
