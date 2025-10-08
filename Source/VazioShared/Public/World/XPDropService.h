#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "XPDropService.generated.h"

UCLASS()
class VAZIOSHARED_API UXPDropService : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XP")
    TSoftClassPtr<AActor> XPDropActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XP")
    float SpawnHeightOffset = 50.f;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "XP")
    void SpawnXPDrop(int32 XPAmount, const FVector& Location);

    virtual void SpawnXPDrop_Implementation(int32 XPAmount, const FVector& Location);

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "XP")
    void OnSpawnXPDrop(int32 XPAmount, const FVector& Location);
};
