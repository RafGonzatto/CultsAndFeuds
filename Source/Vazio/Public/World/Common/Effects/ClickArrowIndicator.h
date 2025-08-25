#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ClickArrowIndicator.generated.h"

class UArrowComponent;

UCLASS()
class VAZIO_API AClickArrowIndicator : public AActor
{
	GENERATED_BODY()
public:
	AClickArrowIndicator();

protected:
	UPROPERTY(VisibleAnywhere)
	UArrowComponent* Arrow;
};