#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"

class UInputMappingContext;
class UInputAction;
class UInteractableComponent;
class AMyCharacter;
class UPlayerHUDWidget;
class USwarmUpgradeSystem;
class UHUDSubsystem;

#include "MyPlayerController.generated.h"

UCLASS()
class VAZIO_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMyPlayerController();

	// NEW: Swarm Level Up
	void TriggerSwarmLevelUp();

	// Networking
	virtual void OnPossess(APawn* InPawn) override;
	virtual void AcknowledgePossession(APawn* P) override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;
    /** ------------- NEW: Ataque ------------- */
	UPROPERTY(EditAnywhere, Category = "Combat") float AttackRadius = 300.f;
	UPROPERTY(EditAnywhere, Category = "Combat") float AttackDamage = 20.f;
	void PerformAreaAttack();

	/** ------------- NEW: HUD (Slate) ------------- */
	void InitializeHUD();

	// NEW: Swarm Upgrade System
	UPROPERTY()
	USwarmUpgradeSystem* SwarmUpgradeSystem;
	
	void InitializeSwarmSystems();

private:
	// Enhanced Input Context e Actions
	UPROPERTY() UInputMappingContext* Mapping = nullptr;
	UPROPERTY() UInputAction* MoveForwardAction = nullptr;
	UPROPERTY() UInputAction* MoveRightAction = nullptr;
	UPROPERTY() UInputAction* ClickAction = nullptr;
	UPROPERTY() UInputAction* Anim1Action = nullptr;
	UPROPERTY() UInputAction* Anim2Action = nullptr;
	UPROPERTY() UInputAction* SprintAction = nullptr;
	UPROPERTY() UInputAction* InteractAction = nullptr; // Enhanced Input: Interact

	// Input Handlers WASD - SISTEMA SIMPLIFICADO
	void OnMoveForward(const FInputActionValue& Value);
	void OnMoveRight(const FInputActionValue& Value);
	void OnMoveForwardCompleted(const FInputActionValue& Value);
	void OnMoveRightCompleted(const FInputActionValue& Value);

	// Sprint - SISTEMA DIRETO
	void OnSprintStart(const FInputActionValue& Value);
	void OnSprintEnd(const FInputActionValue& Value);
	float BaseWalkSpeed = 400.f;
	float SprintMultiplier = 2.0f; // Sprint mais visível
	bool bIsSprinting = false;

	// Click-to-Move - SISTEMA UNIFICADO
	void OnLeftMouseClick();
	void OnLeftMouseRelease();
	UFUNCTION() void OnEnhancedClick(const FInputActionValue& Value);
	void MovePlayerToLocation(FVector TargetLocation);

	// Variáveis de movimento - UMA FONTE DE VERDADE
	UPROPERTY() FVector CurrentMoveTarget;
	UPROPERTY() bool bIsMovingToTarget = false;

	// Atalhos Ctrl+1/Ctrl+2 - SISTEMA DIRETO
	UFUNCTION() void OnAnim1Action(const FInputActionValue& Value);
	UFUNCTION() void OnAnim2Action(const FInputActionValue& Value);

	// Debug
	UFUNCTION() void IncreaseWalkSpeed();
	UFUNCTION() void DecreaseWalkSpeed();

	// SISTEMA DE MOVIMENTO ÚNICO E DEFINITIVO + DEBUG
	float ForwardInput = 0.f;
	float RightInput = 0.f;
	void ProcessMovement(float DeltaTime);

	// SISTEMA DE DEBUG AVANÇADO
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

	// Interact handlers
	// Interaction Controller (centralizado)
	float InteractionRadius = 300.f;      // R
	float InteractionDebounce = 0.12f;    // Δt (s)
	float MovementEpsilon = 1.f;          // ε (cm)
	double LastInteractionQueryTime = 0.0;
	FVector LastInteractionQueryOrigin = FVector::ZeroVector;
	TWeakObjectPtr<class UInteractableComponent> FocusedInteractable;

	void UpdateInteraction(float DeltaTime);
	class UInteractableComponent* FindBestInteractable(const FVector& Origin, float Radius) const;
	void SetFocusedInteractable(class UInteractableComponent* NewTarget);

	UFUNCTION() void OnInteract(const FInputActionValue& Value); // Enhanced Input
	void HandleInteract(); // Legacy/funcional
};
