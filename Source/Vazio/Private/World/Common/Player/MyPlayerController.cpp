#include "World/Common/Player/MyPlayerController.h"
#include "World/Common/Player/MyCharacter.h"
#include "World/Common/Enemy/EnemyHealthComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "World/Common/Effects/ClickArrowIndicator.h"
#include "World/Commom/Interaction/InteractableComponent.h"
#include "World/Common/Interaction/Interactable.h"
#include "UI/Widgets/PlayerHUDWidget.h"

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

#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerInput.h"
#include "InputCoreTypes.h"
#include "InputModifiers.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

// Categoria de log para UI
DEFINE_LOG_CATEGORY_STATIC(LogUI, Log, All);

AMyPlayerController::AMyPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableTouchEvents = false;
	
	// Carregar a classe do widget de HUD durante a construção
	static ConstructorHelpers::FClassFinder<UPlayerHUDWidget> DefaultHUDClass(TEXT("/Game/UI/WBP_PlayerHUD"));
	if (DefaultHUDClass.Succeeded())
	{
		PlayerHUDWidgetClass = DefaultHUDClass.Class;
		UE_LOG(LogUI, Display, TEXT("PlayerHUDWidgetClass carregado no construtor"));
	}
	else
	{
		UE_LOG(LogUI, Warning, TEXT("Não foi possível encontrar /Game/UI/WBP_PlayerHUD no construtor"));
	}
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	IgnoreClickUntilTime = GetWorld() ? GetWorld()->GetTimeSeconds() + 0.25f : 0.f;
	bClickGuardActive = true;

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
		UE_LOG(LogUI, Log, TEXT("[PC] BeginPlay Map=%s IsBattle=%d"), *MapName, bIsBattle ? 1 : 0);
		if (bIsBattle)
		{
			CreatePlayerHUD();
		}
		else
		{
			UE_LOG(LogUI, Verbose, TEXT("[PC] HUD NAO criado (level nao eh Battle_Main)"));
		}
	}
}

void AMyPlayerController::CreatePlayerHUD()
{
	// Verificar se temos uma classe de HUD válida (deve ter sido carregada no construtor)
	if (!PlayerHUDWidgetClass)
	{
		// Tentar carregar diretamente usando LoadClass - uma alternativa segura fora do construtor
		// Nota: isso é mais lento que usar ConstructorHelpers no construtor
		PlayerHUDWidgetClass = LoadClass<UPlayerHUDWidget>(nullptr, TEXT("/Game/UI/WBP_PlayerHUD"));
		
		if (!PlayerHUDWidgetClass)
		{
			UE_LOG(LogUI, Error, TEXT("[PC] Não foi possível carregar a classe do HUD"));
			return;
		}
	}
	
	if (PlayerHUDWidgetClass)
	{
		PlayerHUDWidget = CreateWidget<UPlayerHUDWidget>(this, PlayerHUDWidgetClass);
		if (PlayerHUDWidget)
		{
			PlayerHUDWidget->AddToViewport();
			UE_LOG(LogUI, Display, TEXT("[PC] HUD do player criado e adicionado ao viewport"));
			
			// Tentativa de vincular aos componentes imediatamente
			PlayerHUDWidget->BindToPlayerComponents();
		}
	}
	else
	{
		UE_LOG(LogUI, Error, TEXT("[PC] PlayerHUDWidgetClass é null, não foi possível criar a UI"));
	}
}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
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
		PerformAreaAttack();
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("[PC] Tecla E ignorada (fora de Battle_Main)"));
	}
}

void AMyPlayerController::PerformAreaAttack()
 {
	APawn * P = GetPawn(); if (!P) return;
if (AMyCharacter* C = Cast<AMyCharacter>(P)) {
		C->PerformAreaAttack(AttackRadius, AttackDamage);
		
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

		// APLICAR MOVIMENTO – usando Strength do input (sem turbo nas diagonais)
		const FVector2D Axes(ForwardInput, RightInput);
		const float Strength = FMath::Clamp(Axes.Size(), 0.f, 1.f);
		MyPawn->AddMovementInput(MovementDirection, Strength);
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
	CurrentMoveTarget = TargetLocation;
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
		UE_LOG(LogTemp, Warning, TEXT("[PC] Executando Ctrl+1 - PlayAnim1"));
		MyChar->PlayAnim1();
	}
}

void AMyPlayerController::OnAnim2Action(const FInputActionValue& Value)
{
	if (!Value.Get<bool>()) return;
	if (!(IsInputKeyDown(EKeys::LeftControl) || IsInputKeyDown(EKeys::RightControl))) return;
	
	if (AMyCharacter* MyChar = Cast<AMyCharacter>(GetPawn()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[PC] Executando Ctrl+2 - PlayAnim2"));
		MyChar->PlayAnim2();
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