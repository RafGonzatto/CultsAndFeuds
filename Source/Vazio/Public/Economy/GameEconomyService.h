#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameEconomyService.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UGameEconomyService : public UInterface
{
    GENERATED_BODY()
};

class VAZIO_API IGameEconomyService
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Economy")
    void AddGold(int32 Amount);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Economy")
    void SpawnXPOrbs(int32 TotalXP, const FVector& Location);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Economy")
    int32 GetCurrentGold() const;

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Economy")
    int32 GetCurrentXP() const;
};
