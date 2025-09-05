#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SwarmGameFlow.generated.h"

UCLASS()
class VAZIO_API USwarmGameFlow : public UGameInstanceSubsystem {
  GENERATED_BODY()
public:
  UFUNCTION(BlueprintCallable, Category="Swarm|Flow") void SetReturnPoint(FName Level, const FTransform& T) { ReturnLevel = Level; ReturnTransform = T; }
  UFUNCTION(BlueprintCallable, Category="Swarm|Flow") FName GetReturnLevelName() const { return ReturnLevel; }
  UFUNCTION(BlueprintCallable, Category="Swarm|Flow") FTransform GetReturnTransform() const { return ReturnTransform; }
  UFUNCTION(BlueprintCallable, Category="Swarm|Flow") void EnterArena(FName BattleLevel = TEXT("Battle_Main"));
  UFUNCTION(BlueprintCallable, Category="Swarm|Flow") void ExitArena();
private:
  // Defaults to current City naming in project if not set explicitly
  UPROPERTY() FName ReturnLevel = TEXT("City_Main");
  UPROPERTY() FTransform ReturnTransform; 
};

