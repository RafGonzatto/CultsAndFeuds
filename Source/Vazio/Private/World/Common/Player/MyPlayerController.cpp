#include "World/Common/Player/MyPlayerController.h"
#include "World/Common/Player/MyCharacter.h"
#include "World/Common/Enemy/EnemyHealthComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "World/Common/Effects/ClickArrowIndicator.h"
#include "World/Commom/Interaction/InteractableComponent.h"
#include "World/Common/Interaction/Interactable.h"
#include "UI/Widgets/PlayerHUDWidget.h"
#include "Swarm/Upgrades/SwarmUpgradeSystem.h"
#include "UI/HUD/HUDSubsystem.h"
#include "Engine/World.h"
#include "Net/MatchFlowController.h"
#include "Enemy/EnemySpawnerSubsystem.h"
#include "Enemy/EnemySpawnHelper.h"
#include "Enemy/EnemyBase.h"
#include "GameFramework/Character.h"
#include "EngineUtils.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "World/Battle/BattleGameMode.h"
#include "UI/Widgets/SPauseMenuSlate.h" // Added include for pause menu slate

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Camera/CameraComponent.h"
// Navigation for click-to-move
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Kismet/GameplayStatics.h"
// Helper local para mover player sem SimpleMoveToLocation
static void MovePawnDirect(APawn* Pawn, const FVector& Goal)
{
    if (!Pawn) return; 
    if (UCharacterMovementComponent* CMC = Pawn->FindComponentByClass<UCharacterMovementComponent>())
    {
        const FVector Dir = (Goal - Pawn->GetActorLocation());
        FVector Flat = Dir; Flat.Z = 0.f; 
        if (!Flat.IsNearlyZero())
        {
            Pawn->AddMovementInput(Flat.GetSafeNormal(), 1.f);
        }
    }
}

DEFINE_LOG_CATEGORY_STATIC(LogPlayerUI, Log, All);

AMyPlayerController::AMyPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableTouchEvents = false;
	
	// HUD will be managed by HUDSubsystem instead of Blueprint Widget
	{
		UE_LOG(LogPlayerUI, Warning, TEXT("Não foi possível encontrar /Game/UI/WBP_PlayerHUD no construtor"));
	}
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

    // Make click-to-move responsive immediately
    IgnoreClickUntilTime = 0.f;
    bClickGuardActive = false;

	FInputModeGameOnly GameOnly;
	SetInputMode(GameOnly);
	bShowMouseCursor = true;
	bEnableClickEvents = true;

	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP) return;
	
	UEnhancedInputLocalPlayerSubsystem* Subsys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsys) return;

	Mapping = NewObject<UInputMappingContext>(this, TEXT("IMC_Player"));
	MoveForwardAction = NewObject<UInputAction>(this, TEXT("IA_MoveForward"));
	MoveForwardAction->ValueType = EInputActionValueType::Axis1D;
	MoveRightAction = NewObject<UInputAction>(this, TEXT("IA_MoveRight"));
	MoveRightAction->ValueType = EInputActionValueType::Axis1D;
	ClickAction = NewObject<UInputAction>(this, TEXT("IA_Click"));
	ClickAction->ValueType = EInputActionValueType::Boolean;
	Anim1Action = NewObject<UInputAction>(this, TEXT("IA_Anim1"));
	Anim1Action->ValueType = EInputActionValueType::Boolean;
	Anim2Action = NewObject<UInputAction>(this, TEXT("IA_Anim2"));
	Anim2Action->ValueType = EInputActionValueType::Boolean;
	SprintAction = NewObject<UInputAction>(this, TEXT("IA_Sprint"));
	SprintAction->ValueType = EInputActionValueType::Boolean;
	// Novo: Interact
	InteractAction = NewObject<UInputAction>(this, TEXT("IA_Interact"));
	InteractAction->ValueType = EInputActionValueType::Boolean;
	// Novo: Pause
	PauseAction = NewObject<UInputAction>(this, TEXT("IA_Pause"));
	PauseAction->ValueType = EInputActionValueType::Boolean;

	Mapping->MapKey(MoveForwardAction, EKeys::W);
	{
		FEnhancedActionKeyMapping& S = Mapping->MapKey(MoveForwardAction, EKeys::S);
		S.Modifiers.Add(NewObject<UInputModifierNegate>(this));
	}
	Mapping->MapKey(MoveRightAction, EKeys::D);
	{
		FEnhancedActionKeyMapping& A = Mapping->MapKey(MoveRightAction, EKeys::A);
		A.Modifiers.Add(NewObject<UInputModifierNegate>(this));
	}
	Mapping->MapKey(ClickAction, EKeys::LeftMouseButton);
	Mapping->MapKey(Anim1Action, EKeys::One);
	Mapping->MapKey(Anim2Action, EKeys::Two);
	Mapping->MapKey(SprintAction, EKeys::LeftShift);
	// Novo: mapear Interact no Enhanced Input
	Mapping->MapKey(InteractAction, EKeys::E);
	// Novo: mapear Pause no Enhanced Input
	Mapping->MapKey(PauseAction, EKeys::Escape);

	Subsys->ClearAllMappings();
	Subsys->AddMappingContext(Mapping, 100);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->ClearActionBindings();
		EIC->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AMyPlayerController::OnMoveForward);
		EIC->BindAction(MoveForwardAction, ETriggerEvent::Completed, this, &AMyPlayerController::OnMoveForwardCompleted);
		EIC->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AMyPlayerController::OnMoveRight);
		EIC->BindAction(MoveRightAction, ETriggerEvent::Completed, this, &AMyPlayerController::OnMoveRightCompleted);
		EIC->BindAction(ClickAction, ETriggerEvent::Started, this, &AMyPlayerController::OnEnhancedClick);
		EIC->BindAction(Anim1Action, ETriggerEvent::Started, this, &AMyPlayerController::OnAnim1Action);
		EIC->BindAction(Anim2Action, ETriggerEvent::Started, this, &AMyPlayerController::OnAnim2Action);
		EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &AMyPlayerController::OnSprintStart);
		EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &AMyPlayerController::OnSprintEnd);
		// Novo: binding Enhanced para Interact
		EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AMyPlayerController::OnInteract);
		// Novo: binding Enhanced para Pause
		EIC->BindAction(PauseAction, ETriggerEvent::Started, this, &AMyPlayerController::OnPauseAction);
	}

	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::Equals, IE_Pressed, this, &AMyPlayerController::IncreaseWalkSpeed);
		InputComponent->BindKey(EKeys::Hyphen, IE_Pressed, this, &AMyPlayerController::DecreaseWalkSpeed);

		// Fallback legacy Action Mapping (se existir)
		InputComponent->BindAction("Interact", IE_Pressed, this, &AMyPlayerController::HandleInteract);
	}

	UE_LOG(LogTemp, Warning, TEXT("[PC] PlayerController inicializado - sistema unificado ativo"));
	
	// Criar HUD apenas se estivermos no level de batalha (Battle_Main)
	if (UWorld* W = GetWorld())
	{
		const FString MapName = W->GetMapName(); // Pode vir como UEDPIE_0_Battle_Main em PIE
		const bool bIsBattle = MapName.Contains(TEXT("Battle_Main"));
		UE_LOG(LogPlayerUI, Log, TEXT("[PC] BeginPlay Map=%s IsBattle=%d"), *MapName, bIsBattle ? 1 : 0);
		if (bIsBattle)
		{
			InitializeHUD();
		}
		else
		{
			UE_LOG(LogPlayerUI, Verbose, TEXT("[PC] HUD NAO criado (level nao eh Battle_Main)"));
		}
	}
	InitializeSwarmSystems();
}

void AMyPlayerController::InitializeHUD()
{
	UE_LOG(LogPlayerUI, Display, TEXT("[PC] Inicializando HUD Slate via HUDSubsystem"));
	
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogPlayerUI, Error, TEXT("[PC] GameInstance é null, não é possível obter HUDSubsystem"));
		return;
	}
	
	UHUDSubsystem* HUDSubsystem = GameInstance->GetSubsystem<UHUDSubsystem>();
	if (!HUDSubsystem)
	{
		UE_LOG(LogPlayerUI, Error, TEXT("[PC] HUDSubsystem não encontrado"));
		return;
	}
	
	// Show the Slate HUD
	HUDSubsystem->ShowHUD();
	UE_LOG(LogPlayerUI, Display, TEXT("[PC] HUD Slate inicializado com sucesso"));
}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Bind F1 manually if not already bound (some legacy BP podia estar causando crash)
	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::F1, IE_Pressed, this, &AMyPlayerController::OnDebugF1);
	}
}

void AMyPlayerController::OnDebugF1()
{
	// Executa versão segura do teste – evita qualquer chamada a sistemas potencialmente nulos
	if (!IsValid(this) || !GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("[F1] PlayerController ou World inválido – ignorando"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[F1] Disparado – executando TestCompleteFixes seguro"));
	// Chama a função Exec diretamente (não via console) para garantir mesma lógica
	TestCompleteFixes();
}

void AMyPlayerController::OnInteract(const FInputActionValue& Value)
{
	if (!Value.Get<bool>()) return;
	// GATE: Só permitir ataque especial no mapa Battle_Main
	const bool bIsBattle = GetWorld() && GetWorld()->GetMapName().Contains(TEXT("Battle_Main"));
	if (FocusedInteractable.IsValid())
	{
		HandleInteract();
		return;
	}
	if (bIsBattle)
	{
		UE_LOG(LogTemp, Log, TEXT("Area attack requested"));
		
		// Execute area attack through the character
		if (AMyCharacter* MyChar = Cast<AMyCharacter>(GetPawn()))
		{
			MyChar->Attack();
		}
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("[PC] Tecla E ignorada (fora de Battle_Main)"));
	}
}



void AMyPlayerController::HandleInteract()
{
	// Usar o alvo focado pelo InteractionController
	if (UInteractableComponent* Focus = FocusedInteractable.Get())
	{
		IInteractable::Execute_Interact(Focus, this);
		return;
	}

	// Fallback: selecionar o mais próximo no momento (mesma lógica da busca)
	APawn* P = GetPawn();
	if (!P) return;
	UInteractableComponent* Best = FindBestInteractable(P->GetActorLocation(), InteractionRadius);
	if (Best)
	{
		IInteractable::Execute_Interact(Best, this);
	}
}

// ============================================================================
// SISTEMA DE MOVIMENTO ÚNICO E DEFINITIVO
// ============================================================================

void AMyPlayerController::OnMoveForward(const FInputActionValue& Value)
{
    ForwardInput = Value.Get<float>();
    
    // Cancelar click-to-move imediatamente
    if (FMath::Abs(ForwardInput) > 0.01f)
    {
        if (bIsMovingToTarget)
        {
            bIsMovingToTarget = false;
            UE_LOG(LogTemp, Log, TEXT("[PC] Click-to-move cancelado - input forward"));
        }
        // Stop any nav path following when player takes manual control
        StopMovement();
    }
}

void AMyPlayerController::OnMoveRight(const FInputActionValue& Value)
{
    RightInput = Value.Get<float>();
    
    // Cancelar click-to-move imediatamente
    if (FMath::Abs(RightInput) > 0.01f)
    {
        if (bIsMovingToTarget)
        {
            bIsMovingToTarget = false;
            UE_LOG(LogTemp, Log, TEXT("[PC] Click-to-move cancelado - input right"));
        }
        // Stop any nav path following when player takes manual control
        StopMovement();
    }
}

void AMyPlayerController::OnMoveForwardCompleted(const FInputActionValue& Value)
{
	ForwardInput = 0.f;
}

void AMyPlayerController::OnMoveRightCompleted(const FInputActionValue& Value)
{
	RightInput = 0.f;
}

void AMyPlayerController::OnSprintStart(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		bIsSprinting = true;
		UE_LOG(LogTemp, Log, TEXT("[PC] ? Sprint ATIVO"));
	}
}

void AMyPlayerController::OnSprintEnd(const FInputActionValue& Value)
{
	bIsSprinting = false;
	UE_LOG(LogTemp, Log, TEXT("[PC] ? Sprint DESATIVO"));
}

void AMyPlayerController::ProcessMovement(float DeltaTime)
{
	APawn* MyPawn = GetPawn();
	if (!MyPawn) return;

	// CAPTURAR POSIÇÃO PARA DEBUG
	FVector CurrentPosition = MyPawn->GetActorLocation();
	
	// DETECÇÃO SIMPLES DE TELEPORTE
	if (!LastKnownPosition.IsZero())
	{
		float Distance = FVector::Dist(CurrentPosition, LastKnownPosition);
		if (Distance > 500.0f && Distance < 10000.0f)
		{
			TeleportDetectionCount++;
			UE_LOG(LogTemp, Error, TEXT("?? TELEPORT DETECTADO #%d - Distancia: %.1f"), TeleportDetectionCount, Distance);
			UE_LOG(LogTemp, Error, TEXT("?? De: %s Para: %s"), *LastKnownPosition.ToString(), *CurrentPosition.ToString());
		}
	}
	LastKnownPosition = CurrentPosition;

	FVector MovementDirection = FVector::ZeroVector;
	bool bHasInput = false;

	// PRIORIDADE 1: Input manual (WASD)
	if (FMath::Abs(ForwardInput) > 0.01f || FMath::Abs(RightInput) > 0.01f)
	{
		// Calcular direção baseada na câmera (top-down)
		const FRotator CameraRotation = PlayerCameraManager ? PlayerCameraManager->GetCameraRotation() : FRotator::ZeroRotator;
		
		// Extrair direções da câmera e projetar no plano XY
		const FVector CameraForward = FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::X);
		const FVector CameraRight   = FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::Y);
		
		FVector ForwardDirection = FVector(CameraForward.X, CameraForward.Y, 0.0f).GetSafeNormal();
		FVector RightDirection   = FVector(CameraRight.X,  CameraRight.Y,  0.0f).GetSafeNormal();

		// Patch 1 – respeitar intensidade do input (sem turbo na diagonal)
		const FVector2D Axes(ForwardInput, RightInput);
		const float Strength = FMath::Clamp(Axes.Size(), 0.f, 1.f);
		MovementDirection = (ForwardDirection * Axes.X + RightDirection * Axes.Y).GetSafeNormal();
		
		bHasInput = true;
		
		if (bDebugMovementEnabled)
		{
			UE_LOG(LogTemp, Warning, TEXT("[DEBUG] WASD Input - F:%.2f R:%.2f | Dir:%s | Strength:%.2f"), 
				ForwardInput, RightInput, *MovementDirection.ToString(), Strength);
		}
	}
	// PRIORIDADE 2: Click-to-move (só se não há input manual)
	else if (bIsMovingToTarget)
	{
		const FVector PlayerLocation = MyPawn->GetActorLocation();
		FVector TargetDirection = CurrentMoveTarget - PlayerLocation;
		TargetDirection.Z = 0.0f; // Manter no plano horizontal
		
		const float DistanceToTarget = TargetDirection.Size();
		if (DistanceToTarget < 50.0f) // Chegou ao destino
		{
			bIsMovingToTarget = false;
			MovementDirection = FVector::ZeroVector;
		}
		else
		{
			MovementDirection = TargetDirection.GetSafeNormal();
			bHasInput = true;
		}
	}

	// Aplicar movimento se há direção válida
	if (bHasInput && !MovementDirection.IsNearlyZero())
	{
		// *** CRÍTICO: FORÇAR IGNORE ROOT MOTION ***
		if (USkeletalMeshComponent* MeshComp = MyPawn->GetComponentByClass<USkeletalMeshComponent>())
		{
			if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
			{
				AnimInstance->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
			}
		}

		// Configurar velocidade baseada no sprint
		if (UCharacterMovementComponent* MovementComp = Cast<UCharacterMovementComponent>(MyPawn->GetMovementComponent()))
		{
			const float TargetSpeed = BaseWalkSpeed * (bIsSprinting ? SprintMultiplier : 1.0f);
			MovementComp->MaxWalkSpeed = TargetSpeed; // Aplicação direta
		}

		// APLICAR MOVIMENTO - calcular strength apropriada
		float InputStrength = 1.0f; // Default para click-to-move
		
		// Se há input WASD, usar a intensidade real do input
		if (FMath::Abs(ForwardInput) > 0.01f || FMath::Abs(RightInput) > 0.01f)
		{
			const FVector2D Axes(ForwardInput, RightInput);
			InputStrength = FMath::Clamp(Axes.Size(), 0.f, 1.f);
		}
		
		MyPawn->AddMovementInput(MovementDirection, InputStrength);
	}
	else if (!bIsMovingToTarget)
	{
		// Opcional de depuração – frenagem "seca" quando sem input e sem click-to-move
		if (UCharacterMovementComponent* CMC = Cast<UCharacterMovementComponent>(MyPawn->GetMovementComponent()))
		{
			CMC->StopMovementImmediately();
		}
	}
}

void AMyPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// SISTEMA ÚNICO DE MOVIMENTO
	ProcessMovement(DeltaTime);

	// Atualizar interação centralizada (debounce por movimento/tempo)
	UpdateInteraction(DeltaTime);
}

// ============================================================================
// CLICK-TO-MOVE
// ============================================================================

void AMyPlayerController::OnEnhancedClick(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		// Guard against auto-move triggered by initial click
		if (bClickGuardActive)
		{
			if (UWorld* World = GetWorld())
			{
				if (World->GetTimeSeconds() < IgnoreClickUntilTime)
				{
					UE_LOG(LogTemp, Verbose, TEXT("[PC] Ignorando click inicial (guard)"));
					return;
				}
			}
			bClickGuardActive = false;
		}
		OnLeftMouseClick();
	}
}

void AMyPlayerController::OnLeftMouseClick()
{
	// Guard: não registrar clique até que o tempo atual ultrapasse o tempo de ignore
	if (bClickGuardActive && GetWorld())
	{
		if (GetWorld()->GetTimeSeconds() < IgnoreClickUntilTime)
		{
			UE_LOG(LogTemp, Warning, TEXT("[PC] Clique ignorado pelo guardião de clique"));
			return;
		}
		else
		{
			bClickGuardActive = false; // Desativar guardião após o primeiro clique válido
		}
	}

	FHitResult Hit;
	if (GetHitResultUnderCursor(ECC_Visibility, false, Hit) && Hit.bBlockingHit)
	{
		const FVector Target = Hit.Location;
		UE_LOG(LogTemp, Log, TEXT("[PC] Click-to-move para: %s"), *Target.ToString());
		
		// Spawn visual indicator
		if (APawn* MyPawn = GetPawn(); MyPawn && GetWorld())
		{
			const FVector Direction = (Target - MyPawn->GetActorLocation()).GetSafeNormal();
			const FRotator ArrowRotation = FRotator(0.0f, Direction.Rotation().Yaw, 0.0f);
			
			if (AClickArrowIndicator* Arrow = GetWorld()->SpawnActor<AClickArrowIndicator>(
				AClickArrowIndicator::StaticClass(),
				Target + FVector(0, 0, 5),
				ArrowRotation))
			{
				Arrow->SetLifeSpan(2.0f);
			}
		}
		
		MovePlayerToLocation(Target);
	}
}

void AMyPlayerController::OnLeftMouseRelease()
{
	// Reservado para futuras funcionalidades
}

void AMyPlayerController::MovePlayerToLocation(FVector TargetLocation)
{
    APawn* MyPawn = GetPawn();
    UWorld* World = GetWorld();
    if (!MyPawn || !World)
    {
        CurrentMoveTarget = TargetLocation;
        bIsMovingToTarget = true;
        return;
    }

    StopMovement();

    FVector MoveGoal = TargetLocation;
    if (UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
    {
        FNavLocation Projected;
        const FVector QueryExtent(300.f, 300.f, 500.f);
        if (Nav->ProjectPointToNavigation(TargetLocation, Projected, QueryExtent))
        {
            MoveGoal = Projected.Location;
        }

        if (UNavigationPath* Path = Nav->FindPathToLocationSynchronously(World, MyPawn->GetActorLocation(), MoveGoal))
        {
            if (Path->IsValid())
            {
                if (Path->IsPartial() && Path->PathPoints.Num() > 0)
                {
                    MoveGoal = Path->PathPoints.Last();
                }
                // Em versões onde SimpleMoveToLocation não está disponível para PlayerController, usar fallback manual
                CurrentMoveTarget = MoveGoal;
                bIsMovingToTarget = true; // ProcessMovement fará aproximação
                return;
            }
        }
    }

    CurrentMoveTarget = MoveGoal;
    bIsMovingToTarget = true;
}

// ============================================================================
// TAUNTS - SISTEMA DIRETO SEM FALLBACKS
// ============================================================================

void AMyPlayerController::OnAnim1Action(const FInputActionValue& Value)
{
	if (!Value.Get<bool>()) return;
	if (!(IsInputKeyDown(EKeys::LeftControl) || IsInputKeyDown(EKeys::RightControl))) return;
	
	if (AMyCharacter* MyChar = Cast<AMyCharacter>(GetPawn()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[PC] Executando Ctrl+1 - Debug Animation"));
		// Animation debug function removed
	}
}

void AMyPlayerController::OnAnim2Action(const FInputActionValue& Value)
{
	if (!Value.Get<bool>()) return;
	if (!(IsInputKeyDown(EKeys::LeftControl) || IsInputKeyDown(EKeys::RightControl))) return;
	
	if (AMyCharacter* MyChar = Cast<AMyCharacter>(GetPawn()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[PC] Executando Ctrl+2 - Debug Animation"));
		// Animation debug function removed
	}
}

// ============================================================================
// SISTEMA DE PAUSA
// ============================================================================

void AMyPlayerController::OnPauseAction(const FInputActionValue& Value)
{
	if (!Value.Get<bool>()) return;
	
	UE_LOG(LogTemp, Log, TEXT("[PC] ESC pressed - Toggling pause menu"));
	TogglePauseMenu();
}

void AMyPlayerController::TogglePauseMenu()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[PC] TogglePauseMenu: No valid world"));
		return;
	}

	// Check if pause menu is already visible
	if (SPauseMenuSlate::IsPauseMenuVisible(World))
	{
		UE_LOG(LogTemp, Log, TEXT("[PC] Hiding pause menu"));
		SPauseMenuSlate::HidePauseMenu(World);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[PC] Showing pause menu"));
		SPauseMenuSlate::ShowPauseMenu(World);
	}
}

// ============================================================================
// COMANDOS DE DEBUG SIMPLIFICADOS
// ============================================================================

void AMyPlayerController::DebugMovement()
{
	UE_LOG(LogTemp, Warning, TEXT("=== MOVIMENTO DEBUG ==="));
	UE_LOG(LogTemp, Warning, TEXT("ForwardInput: %.3f"), ForwardInput);
	UE_LOG(LogTemp, Warning, TEXT("RightInput: %.3f"), RightInput);
	UE_LOG(LogTemp, Warning, TEXT("bIsSprinting: %s"), bIsSprinting ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("bIsMovingToTarget: %s"), bIsMovingToTarget ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("TeleportDetections: %d"), TeleportDetectionCount);
}

void AMyPlayerController::DebugAnimations()
{
	UE_LOG(LogTemp, Warning, TEXT("=== ANIMAÇÕES DEBUG ==="));
	
	if (APawn* MyPawn = GetPawn())
	{
		if (USkeletalMeshComponent* MeshComp = MyPawn->GetComponentByClass<USkeletalMeshComponent>())
		{
			UE_LOG(LogTemp, Warning, TEXT("SkeletalMesh: %s"), 
				MeshComp->GetSkeletalMeshAsset() ? *MeshComp->GetSkeletalMeshAsset()->GetName() : TEXT("NULL"));
			UE_LOG(LogTemp, Warning, TEXT("AnimClass: %s"), 
				MeshComp->GetAnimClass() ? *MeshComp->GetAnimClass()->GetName() : TEXT("NULL"));
		}
	}
}

void AMyPlayerController::DebugPositions()
{
	UE_LOG(LogTemp, Warning, TEXT("=== POSIÇÕES DEBUG ==="));
	
	if (APawn* MyPawn = GetPawn())
	{
		FVector ActorLocation = MyPawn->GetActorLocation();
		UE_LOG(LogTemp, Warning, TEXT("Actor Position: %s"), *ActorLocation.ToString());
		
		if (USkeletalMeshComponent* MeshComp = MyPawn->GetComponentByClass<USkeletalMeshComponent>())
		{
			UE_LOG(LogTemp, Warning, TEXT("Mesh Relative: %s"), *MeshComp->GetRelativeLocation().ToString());
			UE_LOG(LogTemp, Warning, TEXT("Mesh World: %s"), *MeshComp->GetComponentLocation().ToString());
		}
	}
}

void AMyPlayerController::ToggleMovementDebug()
{
	bDebugMovementEnabled = !bDebugMovementEnabled;
	UE_LOG(LogTemp, Warning, TEXT("Debug Movement: %s"), bDebugMovementEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void AMyPlayerController::TestEnemySpawn()
{
	UE_LOG(LogTemp, Warning, TEXT("=== TESTING ENEMY SPAWN ==="));
	
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("No world found"));
		return;
	}

	// Check if we're in Battle_Main
	FString MapName = GetWorld()->GetMapName();
	if (!MapName.Contains(TEXT("Battle_Main")))
	{
		UE_LOG(LogTemp, Warning, TEXT("Not in Battle_Main level, spawning test enemies anyway"));
	}

	// Test JSON for quick spawn - TESTING WITH DELAY to avoid timer issues
	FString TestJSON = TEXT(R"({
		"spawnEvents": [
			{
				"time": 0.0,
				"spawns": {
					"NormalEnemy": {
						"count": 3,
						"big": true
					}
				}
			},
			{
				"time": 1.0,
				"spawns": {
					"HeavyEnemy": 2,
					"circle": [
						{
							"type": "RangedEnemy",
							"count": 3,
							"radius": 300
						}
					]
				}
			}
		]
	})");

	// Use the helper function to spawn enemies
	UEnemySpawnHelper::QuickSpawnEnemies(this, TestJSON, FMath::Rand());
	UE_LOG(LogTemp, Warning, TEXT("Started enemy spawn timeline using QuickSpawnEnemies"));
}

void AMyPlayerController::StartTestWave()
{
	UE_LOG(LogTemp, Warning, TEXT("=== STARTING TEST WAVE VIA GAMEMODE ==="));
	
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("No world found"));
		return;
	}

	// Try to get the BattleGameMode and start the test wave
	if (ABattleGameMode* BattleGM = Cast<ABattleGameMode>(GetWorld()->GetAuthGameMode()))
	{
		BattleGM->StartTestWave();
		UE_LOG(LogTemp, Warning, TEXT("Started test wave via BattleGameMode"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not using BattleGameMode, falling back to direct spawn"));
		TestEnemySpawn();
	}
}

void AMyPlayerController::DebugEnemyCount()
{
	UE_LOG(LogTemp, Warning, TEXT("=== DEBUGGING ENEMY COUNT ==="));
	
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("No world found"));
		return;
	}
	
	int32 TotalEnemies = 0;
	int32 VisibleEnemies = 0;
	int32 EnemyBaseCount = 0;
	
	// Count all AEnemyBase actors
	for (TActorIterator<AEnemyBase> ActorItr(World); ActorItr; ++ActorItr)
	{
		AEnemyBase* Enemy = *ActorItr;
		if (Enemy && IsValid(Enemy))
		{
			EnemyBaseCount++;
			FVector Location = Enemy->GetActorLocation();
			bool bHasVisualMesh = Enemy->VisualMesh != nullptr;
			bool bIsVisible = bHasVisualMesh ? Enemy->VisualMesh->IsVisible() : false;
			
			if (bIsVisible) VisibleEnemies++;
			
			UE_LOG(LogTemp, Warning, TEXT("[DEBUG] Enemy %s at location %s - VisualMesh=%s, Visible=%s"), 
				*Enemy->GetName(),
				*Location.ToString(),
				bHasVisualMesh ? TEXT("YES") : TEXT("NO"),
				bIsVisible ? TEXT("YES") : TEXT("NO"));
		}
	}
	
	// Set TotalEnemies to same as EnemyBaseCount for now
	TotalEnemies = EnemyBaseCount;
	
	UE_LOG(LogTemp, Warning, TEXT("[DEBUG] SUMMARY: EnemyBase=%d, Visible=%d, TotalCharacters=%d"), 
		EnemyBaseCount, VisibleEnemies, TotalEnemies);
		
	// Also check player location for reference
	if (APawn* PlayerPawn = GetPawn())
	{
		UE_LOG(LogTemp, Warning, TEXT("[DEBUG] Player location: %s"), *PlayerPawn->GetActorLocation().ToString());
	}
}

void AMyPlayerController::ForceEnemyVisibility()
{
	UE_LOG(LogTemp, Warning, TEXT("=== ENSURING ENEMY VISIBILITY (NORMAL SIZE) ==="));
	
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("No world found"));
		return;
	}
	
	int32 ProcessedEnemies = 0;
	
	for (TActorIterator<AEnemyBase> ActorItr(World); ActorItr; ++ActorItr)
	{
		AEnemyBase* Enemy = *ActorItr;
		if (Enemy && IsValid(Enemy))
		{
			ProcessedEnemies++;
			
			// ENSURE REASONABLE VISIBILITY
			if (Enemy->VisualMesh)
			{
				// Set static mesh to ensure it exists - SAFE RUNTIME LOADING
				if (UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube")))
				{
					Enemy->VisualMesh->SetStaticMesh(CubeMesh);
				}
				
				// NORMAL CHARACTER SCALE - Player-sized
				Enemy->VisualMesh->SetWorldScale3D(FVector(0.9f, 0.9f, 1.6f));
				
				// ENSURE VISIBILITY
				Enemy->VisualMesh->SetVisibility(true);
				Enemy->VisualMesh->SetHiddenInGame(false);
				Enemy->VisualMesh->SetComponentTickEnabled(true);
				
				// Keep color coding but make it less bright
				UMaterialInstanceDynamic* DynamicMat = Enemy->VisualMesh->CreateAndSetMaterialInstanceDynamic(0);
				if (DynamicMat)
				{
					FString ClassName = Enemy->GetClass()->GetName();
					FLinearColor EnemyColor = FLinearColor::Red; // Default
					
					if (ClassName.Contains(TEXT("Heavy"))) EnemyColor = FLinearColor::Blue;
					else if (ClassName.Contains(TEXT("Ranged"))) EnemyColor = FLinearColor::Yellow;
					else if (ClassName.Contains(TEXT("Dash"))) EnemyColor = FLinearColor::Green;
					
					DynamicMat->SetVectorParameterValue("BaseColor", EnemyColor * 0.8f); // Slightly dimmed
					DynamicMat->SetScalarParameterValue("Metallic", 0.1f);
					DynamicMat->SetScalarParameterValue("Roughness", 0.8f);
				}
				
				// KEEP ON GROUND LEVEL - No extreme elevation
				FVector CurrentLoc = Enemy->GetActorLocation();
				if (CurrentLoc.Z > 400.0f || CurrentLoc.Z < 50.0f)
				{
					Enemy->SetActorLocation(FVector(CurrentLoc.X, CurrentLoc.Y, 90.0f)); // Ground level
				}
				
				UE_LOG(LogTemp, Warning, TEXT("[NORMAL] Enemy %s: Location=%s, Scale=%s, Visible=%s"), 
					*Enemy->GetName(),
					*Enemy->GetActorLocation().ToString(),
					*Enemy->VisualMesh->GetComponentScale().ToString(),
					Enemy->VisualMesh->IsVisible() ? TEXT("YES") : TEXT("NO"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[NORMAL] Enemy %s has NO VisualMesh!"), *Enemy->GetName());
			}
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[NORMAL] Processed %d enemies - Look for NORMAL-SIZED colored cubes at ground level!"), ProcessedEnemies);
}

void AMyPlayerController::TestSpawnAndCount()
{
	UE_LOG(LogTemp, Warning, TEXT("=== TEST SPAWN AND IMMEDIATE COUNT WITH AUTO-TELEPORT ==="));
	
	// First, do the spawn
	TestEnemySpawn();
	
	// Create timer handles
	FTimerHandle CountTimer;
	FTimerHandle VisibilityTimer;
	FTimerHandle TeleportTimer;
	
	// Wait a small delay for spawn to process, then count
	GetWorldTimerManager().SetTimer(
		CountTimer,
		this,
		&AMyPlayerController::DebugEnemyCount,
		0.1f,
		false
	);
	
	// Also force visibility after longer delay
	GetWorldTimerManager().SetTimer(
		VisibilityTimer,
		this,
		&AMyPlayerController::ForceEnemyVisibility,
		0.5f,
		false
	);
	
	// Auto-teleport to enemies after they're processed
	GetWorldTimerManager().SetTimer(
		TeleportTimer,
		this,
		&AMyPlayerController::TeleportToEnemies,
		1.0f,
		false
	);
}

void AMyPlayerController::TeleportToEnemies()
{
	UE_LOG(LogTemp, Warning, TEXT("=== TELEPORTING TO ENEMIES ==="));
	
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("No world found"));
		return;
	}
	
	// Find the first enemy
	for (TActorIterator<AEnemyBase> ActorItr(World); ActorItr; ++ActorItr)
	{
		AEnemyBase* Enemy = *ActorItr;
		if (Enemy && IsValid(Enemy))
		{
			FVector EnemyLocation = Enemy->GetActorLocation();
			FVector TeleportLocation = EnemyLocation + FVector(0, 0, 50); // Slightly above enemy
			
			if (APawn* PlayerPawn = GetPawn())
			{
				PlayerPawn->SetActorLocation(TeleportLocation);
				UE_LOG(LogTemp, Warning, TEXT("[TELEPORT] Moved player to %s (near %s)"), 
					*TeleportLocation.ToString(), *Enemy->GetName());
				return;
			}
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[TELEPORT] No enemies found to teleport to"));
}

void AMyPlayerController::TestCloseEnemySpawn()
{
	UE_LOG(LogTemp, Warning, TEXT("=== TESTING NORMAL-SIZED ENEMY SPAWN ==="));
	
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("No world found"));
		return;
	}

	// JSON for normal gameplay spawn - balanced distances and sizes
	FString TestJSON = TEXT(R"({
		"spawnEvents": [
			{
				"time": 0.0,
				"spawns": {
					"NormalEnemy": 2,
					"HeavyEnemy": 1,
					"circle": [
						{
							"type": "RangedEnemy",
							"count": 2,
							"radius": 250
						}
					]
				}
			}
		]
	})");

	// Use the helper function to spawn enemies
	UEnemySpawnHelper::QuickSpawnEnemies(this, TestJSON, FMath::Rand());
	UE_LOG(LogTemp, Warning, TEXT("Started NORMAL-SIZED enemy spawn - player-sized enemies with performance optimization!"));
}

void AMyPlayerController::TestAllEnemyFixes()
{
	UE_LOG(LogTemp, Warning, TEXT("=== TESTING ALL 3 ENEMY FIXES ==="));
	
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("No world found"));
		return;
	}
	
	// First spawn some enemies
	TestCloseEnemySpawn();
	
	// Wait a bit then test all systems
	FTimerHandle TestTimer;
	GetWorldTimerManager().SetTimer(
		TestTimer,
		[this]()
		{
			UWorld* World = GetWorld();
			if (!World) return;
			
			int32 TotalEnemies = 0;
			int32 ColoredEnemies = 0;
			int32 MovingEnemies = 0;
			int32 DamageReadyEnemies = 0;
			
			for (TActorIterator<AEnemyBase> ActorItr(World); ActorItr; ++ActorItr)
			{
				AEnemyBase* Enemy = *ActorItr;
				if (Enemy && IsValid(Enemy))
				{
					TotalEnemies++;
					
					// Test 1: Check colors
					if (Enemy->VisualMesh && Enemy->VisualMesh->GetMaterial(0))
					{
						ColoredEnemies++;
						UE_LOG(LogTemp, Warning, TEXT("[FIX1-COLOR] %s has colored material"), *Enemy->GetName());
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("[FIX1-COLOR] %s missing colored material!"), *Enemy->GetName());
					}
					
					// Test 2: Check movement capability
					if (UCharacterMovementComponent* MovementComp = Enemy->GetCharacterMovement())
					{
						if (MovementComp->MaxWalkSpeed > 0.0f)
						{
							MovingEnemies++;
							UE_LOG(LogTemp, Warning, TEXT("[FIX2-CHASE] %s can move (Speed: %.1f)"), *Enemy->GetName(), MovementComp->MaxWalkSpeed);
						}
						else
						{
							UE_LOG(LogTemp, Error, TEXT("[FIX2-CHASE] %s cannot move!"), *Enemy->GetName());
						}
					}
					
					// Test 3: Check damage system
					if (Enemy->GetCapsuleComponent()->GetGenerateOverlapEvents())
					{
						DamageReadyEnemies++;
						UE_LOG(LogTemp, Warning, TEXT("[FIX3-DAMAGE] %s ready to deal damage"), *Enemy->GetName());
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("[FIX3-DAMAGE] %s cannot deal damage!"), *Enemy->GetName());
					}
				}
			}
			
			UE_LOG(LogTemp, Warning, TEXT("=== ENEMY FIXES SUMMARY ==="));
			UE_LOG(LogTemp, Warning, TEXT("Total Enemies: %d"), TotalEnemies);
			UE_LOG(LogTemp, Warning, TEXT("FIX 1 (Colors): %d/%d working"), ColoredEnemies, TotalEnemies);
			UE_LOG(LogTemp, Warning, TEXT("FIX 2 (Chase): %d/%d working"), MovingEnemies, TotalEnemies);
			UE_LOG(LogTemp, Warning, TEXT("FIX 3 (Damage): %d/%d working"), DamageReadyEnemies, TotalEnemies);
		},
		1.0f,
		false
	);
}

// ============================================================================
// DEBUG
// ============================================================================

void AMyPlayerController::IncreaseWalkSpeed()
{
	BaseWalkSpeed += 50.f;
	UE_LOG(LogTemp, Warning, TEXT("[PC] BaseWalkSpeed=%.1f"), BaseWalkSpeed);
}

void AMyPlayerController::DecreaseWalkSpeed()
{
	BaseWalkSpeed = FMath::Max(100.f, BaseWalkSpeed - 50.f);
	UE_LOG(LogTemp, Warning, TEXT("[PC] BaseWalkSpeed=%.1f"), BaseWalkSpeed);
}

void AMyPlayerController::UpdateInteraction(float DeltaTime)
{
	UWorld* World = GetWorld();
	APawn* P = GetPawn();
	if (!World || !P) return;

	const FVector Origin = P->GetActorLocation();
	const double Now = World->GetTimeSeconds();
	const bool bMovedEnough = FVector::DistSquared(Origin, LastInteractionQueryOrigin) > FMath::Square(MovementEpsilon);
	const bool bDebounced = (Now - LastInteractionQueryTime) >= InteractionDebounce;
	if (!bMovedEnough && !bDebounced) return;

	LastInteractionQueryTime = Now;
	LastInteractionQueryOrigin = Origin;

	UInteractableComponent* Best = FindBestInteractable(Origin, InteractionRadius);
	SetFocusedInteractable(Best);
}

UInteractableComponent* AMyPlayerController::FindBestInteractable(const FVector& Origin, float Radius) const
{
	UInteractableComponent* Best = nullptr;
	float BestDistSq = FMath::Square(Radius);

	for (const TWeakObjectPtr<UInteractableComponent>& Weak : UInteractableComponent::GRegistry)
	{
		if (UInteractableComponent* Comp = Weak.Get())
		{
			if (!Comp->IsAvailable()) continue;
			const float DistSq = FVector::DistSquared(Origin, Comp->GetInteractLocation());
			if (DistSq <= BestDistSq)
			{
				Best = Comp;
				BestDistSq = DistSq;
			}
		}
	}
	return Best;
}
// Adicione estas novas funções no final do arquivo:
void AMyPlayerController::InitializeSwarmSystems()
{
    if (!GetWorld())
    {
        return;
    }
    
    FString LevelName = GetWorld()->GetMapName();
    if (LevelName.Contains(TEXT("Battle_Main")))
    {
        // Create and initialize upgrade system
        SwarmUpgradeSystem = NewObject<USwarmUpgradeSystem>(this);
        if (SwarmUpgradeSystem)
        {
            SwarmUpgradeSystem->Initialize(this);
            UE_LOG(LogTemp, Warning, TEXT("Swarm Upgrade System initialized for Battle_Main"));
        }
    }
}

void AMyPlayerController::TriggerSwarmLevelUp()
{
    if (SwarmUpgradeSystem)
    {
        SwarmUpgradeSystem->TriggerLevelUp();
    }
}
void AMyPlayerController::SetFocusedInteractable(UInteractableComponent* NewTarget)
{
	UInteractableComponent* Old = FocusedInteractable.Get();
	if (Old == NewTarget) return;

	// Ocultar prompt do antigo
	if (Old)
	{
		Old->ShowPrompt(false);
	}

	UE_LOG(LogTemp, Warning, TEXT("[PC][Interact] Focus change: %s -> %s"), Old ? *Old->GetOwner()->GetName() : TEXT("None"), NewTarget ? *NewTarget->GetOwner()->GetName() : TEXT("None"));
	FocusedInteractable = NewTarget;

	// Mostrar prompt do novo e ocultar os demais para evitar ruído visual
	if (NewTarget)
	{
		NewTarget->ShowPrompt(true);
		for (const TWeakObjectPtr<UInteractableComponent>& Weak : UInteractableComponent::GRegistry)
		{
			if (UInteractableComponent* Comp = Weak.Get())
			{
				if (Comp != NewTarget)
				{
					Comp->ShowPrompt(false);
				}
			}
		}
	}
}

void AMyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	UE_LOG(LogTemp, Warning, TEXT("[PC] OnPossess: %s"), InPawn ? *InPawn->GetName() : TEXT("None"));
	
	// If we're the host and this is Battle_Main, transition to InMatchHost
	if (HasAuthority() && GetWorld())
	{
		const FString MapName = GetWorld()->GetMapName();
		if (MapName.Contains(TEXT("Battle_Main")))
		{
			if (UGameInstance* GI = GetGameInstance())
			{
				if (UMatchFlowController* MatchFlow = GI->GetSubsystem<UMatchFlowController>())
				{
					// Set to InMatchHost state and hide loading screen after a short delay
					FTimerHandle DelayTimer;
					GetWorld()->GetTimerManager().SetTimer(DelayTimer, [MatchFlow]()
					{
						MatchFlow->HideLoadingScreen();
					}, 1.5f, false);
				}
			}
		}
	}
}

void AMyPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);
	
	UE_LOG(LogTemp, Warning, TEXT("[PC] AcknowledgePossession: %s"), P ? *P->GetName() : TEXT("None"));
	
	// If we're a client and acknowledged pawn in Battle_Main, hide loading screen
	if (!HasAuthority() && GetWorld())
	{
		const FString MapName = GetWorld()->GetMapName();
		if (MapName.Contains(TEXT("Battle_Main")))
		{
			if (UGameInstance* GI = GetGameInstance())
			{
				if (UMatchFlowController* MatchFlow = GI->GetSubsystem<UMatchFlowController>())
				{
					// Small delay to ensure everything is ready
					FTimerHandle DelayTimer;
					GetWorld()->GetTimerManager().SetTimer(DelayTimer, [MatchFlow]()
					{
						MatchFlow->HideLoadingScreen();
					}, 1.0f, false);
				}
			}
		}
	}
}

void AMyPlayerController::TestCompleteFixes()
{
	UE_LOG(LogTemp, Warning, TEXT("=== CRASH-SAFE ENEMY TEST STARTED ==="));
	
	UWorld* World = GetWorld();
	if (!World || !IsValid(World))
	{
		UE_LOG(LogTemp, Error, TEXT("No valid world found"));
		return;
	}
	
	// IMMEDIATE TEST - No timers to avoid crashes
	UE_LOG(LogTemp, Warning, TEXT("TESTING EXISTING ENEMIES..."));
	
	int32 TotalEnemies = 0;
	int32 ValidEnemies = 0;
	int32 VisibleEnemies = 0;
	int32 MovingEnemies = 0;
	int32 TickingEnemies = 0;
	
	// SAFE iteration through enemies
	for (TActorIterator<AEnemyBase> ActorItr(World); ActorItr; ++ActorItr)
	{
		AEnemyBase* Enemy = *ActorItr;
		if (!Enemy)
		{
			continue;
		}
		
		TotalEnemies++;
		
		// Test validity
		if (IsValid(Enemy))
		{
			ValidEnemies++;
			
			// Test visibility (with null checks)
			if (Enemy->VisualMesh && IsValid(Enemy->VisualMesh) && Enemy->VisualMesh->IsVisible())
			{
				VisibleEnemies++;
			}
			
			// Test movement capability (with null checks)
			UCharacterMovementComponent* MovementComp = Enemy->GetCharacterMovement();
			if (MovementComp && IsValid(MovementComp) && MovementComp->MaxWalkSpeed > 0.0f)
			{
				MovingEnemies++;
			}
			
			// Test tick capability
			if (Enemy->IsActorTickEnabled())
			{
				TickingEnemies++;
			}
			
			UE_LOG(LogTemp, Warning, TEXT("[SAFE] Enemy %s: Valid=%s, Visible=%s, CanMove=%s, Ticking=%s"), 
				*Enemy->GetName(),
				IsValid(Enemy) ? TEXT("YES") : TEXT("NO"),
				(Enemy->VisualMesh && Enemy->VisualMesh->IsVisible()) ? TEXT("YES") : TEXT("NO"),
				(MovementComp && MovementComp->MaxWalkSpeed > 0.0f) ? TEXT("YES") : TEXT("NO"),
				Enemy->IsActorTickEnabled() ? TEXT("YES") : TEXT("NO"));
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("=== SAFE TEST RESULTS ==="));
	UE_LOG(LogTemp, Warning, TEXT("Total: %d, Valid: %d, Visible: %d, Moving: %d, Ticking: %d"), 
		TotalEnemies, ValidEnemies, VisibleEnemies, MovingEnemies, TickingEnemies);
	
	// SPAWN NEW ENEMIES SAFELY
	if (TotalEnemies == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No enemies found, spawning test enemies..."));
		TestCloseEnemySpawn();
	}
	
	// Test player system safely
	if (APawn* PlayerPawn = GetPawn())
	{
		if (IsValid(PlayerPawn))
		{
			UE_LOG(LogTemp, Warning, TEXT("Player: Valid and active at %s"), *PlayerPawn->GetActorLocation().ToString());
			
			if (AMyCharacter* MyChar = Cast<AMyCharacter>(PlayerPawn))
			{
				if (MyChar->GetHealthComponent())
				{
					float CurrentHP = MyChar->GetHealthComponent()->GetCurrentHealth();
					float MaxHP = MyChar->GetHealthComponent()->GetMaxHealth();
					UE_LOG(LogTemp, Warning, TEXT("Player Health: %.1f/%.1f HP"), CurrentHP, MaxHP);
				}
			}
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("=== CRASH-SAFE TEST COMPLETED ==="));
}
