#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArenaReturnPositioner.generated.h"

UCLASS()
class VAZIO_API AArenaReturnPositioner : public AActor
{
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
};

