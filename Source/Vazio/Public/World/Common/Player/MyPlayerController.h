#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "UI/Widgets/SPauseMenuSlate.h" // Added include to ensure visibility of SPauseMenuSlate static API

class UInputMappingContext;
class UInputAction;
class UInteractableComponent;
class AMyCharacter;
class UPlayerHUDWidget;
class USpringArmComponent;
class UCameraComponent;
class UHUDSubsystem;
class UBossAutoTestSubsystem;
class UPlayerHealthComponent;
class UXPComponent;

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

	// Upgrade system now handled by UUpgradeSubsystem (World Subsystem)
	// Access via: GetWorld()->GetSubsystem<UUpgradeSubsystem>()
	
	void InitializeSwarmSystems();

	/** ------------- BOSS TESTING SYSTEM ------------- */
	UPROPERTY()
	UBossAutoTestSubsystem* BossTestSubsystem;
	
	void InitializeBossTestSystem();
	bool bBossTestModeActive = false;

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
	UPROPERTY() UInputAction* PauseAction = nullptr; // Enhanced Input: Pause (ESC)
	
	// Boss Testing Actions
	UPROPERTY() UInputAction* BossTest1Action = nullptr; // Burrower
	UPROPERTY() UInputAction* BossTest2Action = nullptr; // Void Queen
	UPROPERTY() UInputAction* BossTest3Action = nullptr; // Fallen Warlord
	UPROPERTY() UInputAction* BossTest4Action = nullptr; // Hybrid Demon
	UPROPERTY() UInputAction* BossTest5Action = nullptr; // All Bosses
	UPROPERTY() UInputAction* BossTest0Action = nullptr; // Stop
	UPROPERTY() UInputAction* BossTestToggleAction = nullptr; // F12 - Toggle

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

	// Pause Menu - SISTEMA DE PAUSA
	UFUNCTION() void OnPauseAction(const FInputActionValue& Value);
	void TogglePauseMenu();

	// Boss Testing Input Handlers
	UFUNCTION() void OnBossTest1(const FInputActionValue& Value);
	UFUNCTION() void OnBossTest2(const FInputActionValue& Value);
	UFUNCTION() void OnBossTest3(const FInputActionValue& Value);
	UFUNCTION() void OnBossTest4(const FInputActionValue& Value);
	UFUNCTION() void OnBossTest5(const FInputActionValue& Value);
	UFUNCTION() void OnBossTest0(const FInputActionValue& Value);
	UFUNCTION() void OnBossTestToggle(const FInputActionValue& Value);
	
	// Boss Testing Helper

	// -------- HUD bindings to player components --------
	void BindHUDToPlayerComponents();
	void UnbindHUDFromPlayerComponents();
	UFUNCTION() void HandleHealthChanged(float Current, float Max);
	UFUNCTION() void HandleXPChanged(int32 CurrentXP, int32 XPToNextLevel);
	UFUNCTION() void HandleLevelChanged(int32 NewLevel);

	TWeakObjectPtr<UPlayerHealthComponent> BoundHealthComp;
	TWeakObjectPtr<UXPComponent> BoundXPComp;
	FDelegateHandle HealthChangedHandle;
	bool IsBossTestingAvailable() const;

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

	UFUNCTION(Exec)
	void TestEnemySpawn();

	UFUNCTION(Exec)
	void StartTestWave();

	UFUNCTION(Exec)
	void DebugEnemyCount();

	UFUNCTION(Exec)
	void ForceEnemyVisibility();

	UFUNCTION(Exec)
	void TestSpawnAndCount();

	UFUNCTION(Exec)
	void TeleportToEnemies();

	UFUNCTION(Exec)
	void TestCloseEnemySpawn();
	
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
	
	// Debug function to test all enemy fixes
	UFUNCTION(CallInEditor, Category="Debug")
	void TestAllEnemyFixes();
	
	// New comprehensive test function
	UFUNCTION(Exec, CallInEditor, Category="Debug")
	void TestCompleteFixes();

	// Consome a tecla F1 (evitar crash vindo de Blueprints antigos) - DEPRECATED, agora usa F12
	void OnDebugF1();

private:
	// Helper method for boss test validation
	bool ValidateBossTestAction(const FInputActionValue& Value, const FString& ActionName);
};
