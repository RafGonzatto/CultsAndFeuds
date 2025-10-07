// AutoRespawnComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AutoRespawnComponent.generated.h"

class APlayerStart;
class ATriggerBox;

UCLASS(ClassGroup = (Gameplay), meta = (BlueprintSpawnableComponent))
class VAZIO_API UAutoRespawnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAutoRespawnComponent();

	// Se setado, usa esse ator como ponto de respawn (sobrepõe qualquer busca por PlayerStart)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
	TSoftObjectPtr<AActor> OverrideSpawnPoint;

	// Tag do PlayerStart preferido (ex.: "CitySpawn", "DungeonSpawn"...)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
	FName PreferredSpawnTag = TEXT("CitySpawn");

	// Intervalo de checagem (Hz ~ 5 é suficiente e leve)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float CheckIntervalSeconds = 0.2f;

	// Limites numéricos (usados se NÃO houver volume na cena)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
	bool bUseNumericZLimits = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn", meta = (EditCondition = "bUseNumericZLimits"))
	float MinZ = -10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn", meta = (EditCondition = "bUseNumericZLimits"))
	float MaxZ = 200000.f;

	// Se true, tenta detectar um ATriggerBox com tag "RespawnBounds" e usar seu volume como limites
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
	bool bTryFindBoundsVolume = true;

protected:
	virtual void BeginPlay() override;

private:
	FTimerHandle CheckHandle;
	TWeakObjectPtr<AActor> CachedSpawnPoint;
	TWeakObjectPtr<ATriggerBox> CachedBounds;

	void StartChecks();
	void CheckAndRecover();
	bool IsOutOfBounds(const FVector& Loc) const;
	AActor* ResolveSpawnPoint() const;
	bool TeleportOwnerToSpawn(AActor* Spawn) const;
};
