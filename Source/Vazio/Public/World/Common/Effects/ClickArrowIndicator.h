#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

class UArrowComponent;

#include "ClickArrowIndicator.generated.h"

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