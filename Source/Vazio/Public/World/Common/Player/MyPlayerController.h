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
	virtual void Tick(float DeltaTime) override;

private:
	// Enhanced Input Context e Actions
	UPROPERTY() UInputMappingContext* Mapping = nullptr;
	UPROPERTY() UInputAction* MoveForwardAction = nullptr;
	UPROPERTY() UInputAction* MoveRightAction = nullptr;
	UPROPERTY() UInputAction* ClickAction = nullptr;
	UPROPERTY() UInputAction* Anim1Action = nullptr;
	UPROPERTY() UInputAction* Anim2Action = nullptr;
	UPROPERTY() UInputAction* SprintAction = nullptr;

	// Input Handlers WASD - SISTEMA SIMPLIFICADO
	void OnMoveForward(const FInputActionValue& Value);
	void OnMoveRight(const FInputActionValue& Value);
	void OnMoveForwardCompleted(const FInputActionValue& Value);
	void OnMoveRightCompleted(const FInputActionValue& Value);

	// Sprint - SISTEMA DIRETO
	void OnSprintStart(const FInputActionValue& Value);
	void OnSprintEnd(const FInputActionValue& Value);
	float BaseWalkSpeed = 400.f;
	float SprintMultiplier = 2.0f; // Sprint mais vis�vel
	bool bIsSprinting = false;

	// Click-to-Move - SISTEMA UNIFICADO
	void OnLeftMouseClick();
	void OnLeftMouseRelease();
	UFUNCTION() void OnEnhancedClick(const FInputActionValue& Value);
	void MovePlayerToLocation(FVector TargetLocation);

	// Vari�veis de movimento - UMA FONTE DE VERDADE
	UPROPERTY() FVector CurrentMoveTarget;
	UPROPERTY() bool bIsMovingToTarget = false;

	// Atalhos Ctrl+1/Ctrl+2 - SISTEMA DIRETO
	UFUNCTION() void OnAnim1Action(const FInputActionValue& Value);
	UFUNCTION() void OnAnim2Action(const FInputActionValue& Value);

	// Debug
	UFUNCTION() void IncreaseWalkSpeed();
	UFUNCTION() void DecreaseWalkSpeed();

	// SISTEMA DE MOVIMENTO �NICO E DEFINITIVO + DEBUG
	float ForwardInput = 0.f;
	float RightInput = 0.f;
	void ProcessMovement(float DeltaTime);

	// SISTEMA DE DEBUG AVAN�ADO
	FVector LastKnownPosition;
	FVector LastFramePosition;
	float DebugTimer = 0.f;
	int32 TeleportDetectionCount = 0;
	
	UFUNCTION(Exec)
	void DebugMovement();
	
	UFUNCTION(Exec)
	void DebugAnimations();
	
	UFUNCTION(Exec)
	void DebugPositions();
	
	UFUNCTION(Exec)
	void ToggleMovementDebug();
	
	bool bDebugMovementEnabled = false;
	void LogMovementState();
	void DetectTeleports();

	// Click-to-Move guard to ignore initial viewport click
	float IgnoreClickUntilTime = 0.f;
	bool bClickGuardActive = false;
};
