#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

class APlayerController;

UINTERFACE(BlueprintType)
class VAZIO_API UInteractable : public UInterface
{
	GENERATED_BODY()
};

class VAZIO_API IInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	void Interact(APlayerController* InteractingPC);
};
