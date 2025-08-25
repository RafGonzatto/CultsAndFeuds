#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "MyPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class VAZIO_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMyPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	// Criados em runtime (sem uassets)
	UPROPERTY() UInputMappingContext* Mapping = nullptr;
	UPROPERTY() UInputAction* MoveForwardAction = nullptr;
	UPROPERTY() UInputAction* MoveRightAction = nullptr;
	UPROPERTY() UInputAction* LookYawAction = nullptr;

	// Handlers
	void OnMoveForward(const FInputActionValue& Value);
	void OnMoveRight(const FInputActionValue& Value);
	void OnLookYaw(const FInputActionValue& Value);
};
