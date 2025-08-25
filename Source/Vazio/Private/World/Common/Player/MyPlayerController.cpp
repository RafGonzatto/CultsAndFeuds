#include "World/Common/Player/MyPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerInput.h"
#include "InputCoreTypes.h"
#include "InputModifiers.h"
#include "DrawDebugHelpers.h"
#include "World/Common/Effects/ClickArrowIndicator.h"

AMyPlayerController::AMyPlayerController()
{
	PrimaryActorTick.bCanEverTick = true; // garantir Tick

	bShowMouseCursor = true; // ATIVAR cursor para click-to-move
	bEnableClickEvents = true; // ATIVAR click events
	bEnableTouchEvents = false;
	
	UE_LOG(LogTemp, Warning, TEXT("[MyPlayerController] Construtor - Cursor HABILITADO para click-to-move"));
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("[MyPlayerController] BeginPlay iniciado - configurando Enhanced Input"));

	// Garantir foco de input no jogo (não UI)
	FInputModeGameOnly GameOnly;
	SetInputMode(GameOnly);
	bShowMouseCursor = true; // manter visível para click-to-move
	bEnableClickEvents = true;

	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP) 
	{
		UE_LOG(LogTemp, Error, TEXT("[MyPlayerController] LocalPlayer é NULL"));
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsys) 
	{
		UE_LOG(LogTemp, Error, TEXT("[MyPlayerController] Enhanced Input Subsystem é NULL"));
		return;
	}

	// Criar Mapping Context e Ações em runtime (APENAS MOVIMENTO)
	Mapping = NewObject<UInputMappingContext>(this, TEXT("IMC_CityMovement"));
	MoveForwardAction = NewObject<UInputAction>(this, TEXT("IA_MoveForward"));
	MoveRightAction = NewObject<UInputAction>(this, TEXT("IA_MoveRight"));

	MoveForwardAction->ValueType = EInputActionValueType::Axis1D;
	MoveRightAction->ValueType = EInputActionValueType::Axis1D;

	{
		Mapping->MapKey(MoveForwardAction, EKeys::W);
		FEnhancedActionKeyMapping& S = Mapping->MapKey(MoveForwardAction, EKeys::S);
		UInputModifierNegate* NegateS = NewObject<UInputModifierNegate>(this);
		S.Modifiers.Add(NegateS);
	}
	{
		Mapping->MapKey(MoveRightAction, EKeys::D);
		FEnhancedActionKeyMapping& A = Mapping->MapKey(MoveRightAction, EKeys::A);
		UInputModifierNegate* NegateA = NewObject<UInputModifierNegate>(this);
		A.Modifiers.Add(NegateA);
	}

	// Aplicar contexto com prioridade alta
	Subsys->ClearAllMappings();
	Subsys->AddMappingContext(Mapping, 100);
	UE_LOG(LogTemp, Warning, TEXT("[MyPlayerController] MappingContext aplicado (prio=100)"));

	// IMPORTANTE: Fazer os binds DEPOIS de criar as Actions (SetupInputComponent roda antes)
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->ClearActionBindings();
		EIC->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AMyPlayerController::OnMoveForward);
		EIC->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AMyPlayerController::OnMoveRight);
		UE_LOG(LogTemp, Warning, TEXT("[MyPlayerController] BindAction refeito em BeginPlay"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MyPlayerController] InputComponent não é EnhancedInputComponent"));
	}

	EnableInput(this);

	// Timer de teste
	FTimerHandle TestInputTimer;
	GetWorld()->GetTimerManager().SetTimer(TestInputTimer, this, &AMyPlayerController::TestInputSystem, 1.0f, false);
}

void AMyPlayerController::TestInputSystem()
{
	UE_LOG(LogTemp, Error, TEXT("========================================"));
	UE_LOG(LogTemp, Error, TEXT("[MyPlayerController] *** TESTE DE SISTEMAS DE INPUT ***"));
	UE_LOG(LogTemp, Error, TEXT("1. Pressione WASD para testar Enhanced Input"));
	UE_LOG(LogTemp, Error, TEXT("2. Clique na tela para testar Click-to-Move"));
	UE_LOG(LogTemp, Error, TEXT("========================================"));
	
	// Verificar se o pawn está correto
	if (APawn* MyPawn = GetPawn())
	{
		UE_LOG(LogTemp, Warning, TEXT("[MyPlayerController] Pawn controlado: %s na posição: %s"), 
			*MyPawn->GetName(), 
			*MyPawn->GetActorLocation().ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MyPlayerController] NENHUM PAWN está sendo controlado!"));
	}

	// Verificar se o input component existe
	if (InputComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MyPlayerController] InputComponent existe: %s"), *InputComponent->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MyPlayerController] InputComponent é NULL!"));
	}
	
	// Verificar Enhanced Input
	if (Mapping && MoveForwardAction && MoveRightAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MyPlayerController] Enhanced Input Actions criadas corretamente"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MyPlayerController] Problema com Enhanced Input Actions!"));
	}
}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AMyPlayerController::OnMoveForward);
		EIC->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AMyPlayerController::OnMoveRight);
		UE_LOG(LogTemp, Warning, TEXT("[MyPlayerController] Enhanced Input bindings criados - MOVIMENTO WASD"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MyPlayerController] Enhanced Input Component não encontrado"));
	}

	// Bind direto de tecla (sem precisar de Action Mapping no Project Settings)
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AMyPlayerController::OnLeftMouseClick);
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &AMyPlayerController::OnLeftMouseRelease);
	UE_LOG(LogTemp, Warning, TEXT("[MyPlayerController] Click-to-Move binding direto (BindKey) adicionado"));
}

void AMyPlayerController::OnMoveForward(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();
	UE_LOG(LogTemp, Error, TEXT("[MyPlayerController] *** INPUT W/S DETECTADO: %f ***"), Axis);

	if (APawn* ControlledPawn = GetPawn())
	{
		// Direções alinhadas à câmera (top-down), projetadas no plano XY
		const FRotator CamRot = PlayerCameraManager ? PlayerCameraManager->GetCameraRotation() : FRotator::ZeroRotator;
		FVector ForwardDir = FRotationMatrix(CamRot).GetUnitAxis(EAxis::X);
		ForwardDir.Z = 0.f;
		ForwardDir.Normalize();
		
		ControlledPawn->AddMovementInput(ForwardDir, Axis);
		if (FMath::Abs(Axis) > 0.01f)
		{
			UE_LOG(LogTemp, Error, TEXT("[MyPlayerController] Player MOVEU FRENTE/TRÁS - Pos: %s"), *ControlledPawn->GetActorLocation().ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MyPlayerController] INPUT RECEBIDO mas NENHUM PAWN para controlar!"));
	}
}

void AMyPlayerController::OnMoveRight(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();
	UE_LOG(LogTemp, Error, TEXT("[MyPlayerController] *** INPUT A/D DETECTADO: %f ***"), Axis);

	if (APawn* ControlledPawn = GetPawn())
	{
		// Direções alinhadas à câmera (top-down), projetadas no plano XY
		const FRotator CamRot = PlayerCameraManager ? PlayerCameraManager->GetCameraRotation() : FRotator::ZeroRotator;
		FVector RightDir = FRotationMatrix(CamRot).GetUnitAxis(EAxis::Y);
		RightDir.Z = 0.f;
		RightDir.Normalize();
		
		ControlledPawn->AddMovementInput(RightDir, Axis);
		if (FMath::Abs(Axis) > 0.01f)
		{
			UE_LOG(LogTemp, Error, TEXT("[MyPlayerController] Player MOVEU ESQ/DIR - Pos: %s"), *ControlledPawn->GetActorLocation().ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MyPlayerController] INPUT RECEBIDO mas NENHUM PAWN para controlar!"));
	}
}

void AMyPlayerController::OnLeftMouseClick()
{
	UE_LOG(LogTemp, Error, TEXT("[MyPlayerController] *** MOUSE CLICK DETECTADO! ***"));
	FHitResult HitResult;
	bool bHit = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, HitResult);
	if (bHit && HitResult.bBlockingHit)
	{
		FVector TargetLocation = HitResult.Location;
		UE_LOG(LogTemp, Error, TEXT("[MyPlayerController] Click no mundo em: %s"), *TargetLocation.ToString());
		
		// Seta/indicador no ponto clicado, virado para a direção do movimento
		APawn* MyPawn = GetPawn();
		FVector From = MyPawn ? MyPawn->GetActorLocation() : TargetLocation;
		FVector Dir = (TargetLocation - From);
		Dir.Z = 0.f;
		FRotator ArrowYaw(0.f, Dir.Rotation().Yaw, 0.f);
		AClickArrowIndicator* Arrow = GetWorld()->SpawnActor<AClickArrowIndicator>(AClickArrowIndicator::StaticClass(), TargetLocation + FVector(0,0,2), ArrowYaw);
		if (Arrow)
		{
			Arrow->SetLifeSpan(1.5f);
		}
		
		// Move imediatamente
		MovePlayerToLocation(TargetLocation);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[MyPlayerController] Click não atingiu nada no mundo"));
	}
}

void AMyPlayerController::OnLeftMouseRelease()
{
	// Opcional: pode ser usado para parar movimento ou outros efeitos
	UE_LOG(LogTemp, Warning, TEXT("[MyPlayerController] Mouse liberado"));
}

void AMyPlayerController::MovePlayerToLocation(FVector TargetLocation)
{
	APawn* MyPawn = GetPawn();
	if (!MyPawn) 
	{
		UE_LOG(LogTemp, Error, TEXT("[MyPlayerController] Nenhum Pawn para mover!"));
		return;
	}

	// Calcular direção do movimento
	FVector PlayerLocation = MyPawn->GetActorLocation();
	FVector Direction = (TargetLocation - PlayerLocation).GetSafeNormal();
	
	// Manter a altura do player (não mover em Z)
	Direction.Z = 0.0f;
	
	UE_LOG(LogTemp, Error, TEXT("[MyPlayerController] Movendo player de %s para %s"), 
		*PlayerLocation.ToString(), 
		*TargetLocation.ToString());

	// Aplicar movimento diretamente
	if (ACharacter* MyCharacter = Cast<ACharacter>(MyPawn))
	{
		// Usar AddMovementInput para movimento suave
		MyCharacter->AddMovementInput(Direction, 1.0f);
		
		// Salvar o target para movimento contínuo
		CurrentMoveTarget = TargetLocation;
		bIsMovingToTarget = true;
		
		UE_LOG(LogTemp, Error, TEXT("[MyPlayerController] *** INICIANDO MOVIMENTO PARA TARGET ***"));
	}
}

void AMyPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Fallback WASD manual alinhado com a câmera
	float AxisY = 0.f;
	float AxisX = 0.f;
	if (IsInputKeyDown(EKeys::W)) AxisY += 1.f;
	if (IsInputKeyDown(EKeys::S)) AxisY -= 1.f;
	if (IsInputKeyDown(EKeys::D)) AxisX += 1.f;
	if (IsInputKeyDown(EKeys::A)) AxisX -= 1.f;
	
	if (APawn* MyPawn = GetPawn())
	{
		const FRotator CamRot = PlayerCameraManager ? PlayerCameraManager->GetCameraRotation() : FRotator::ZeroRotator;
		FVector ForwardDir = FRotationMatrix(CamRot).GetUnitAxis(EAxis::X); ForwardDir.Z = 0.f; ForwardDir.Normalize();
		FVector RightDir = FRotationMatrix(CamRot).GetUnitAxis(EAxis::Y); RightDir.Z = 0.f; RightDir.Normalize();
		
		if (FMath::Abs(AxisY) > 0.01f)
		{
			MyPawn->AddMovementInput(ForwardDir, AxisY);
		}
		if (FMath::Abs(AxisX) > 0.01f)
		{
			MyPawn->AddMovementInput(RightDir, AxisX);
		}
	}
	
	// Movimento contínuo do click-to-move
	if (bIsMovingToTarget)
	{
		APawn* MyPawn2 = GetPawn();
		if (MyPawn2)
		{
			FVector PlayerLocation = MyPawn2->GetActorLocation();
			FVector TargetDirection = (CurrentMoveTarget - PlayerLocation);
			TargetDirection.Z = 0.0f;
			float DistanceToTarget = TargetDirection.Size();
			if (DistanceToTarget < 50.0f)
			{
				bIsMovingToTarget = false;
			}
			else
			{
				FVector Direction = TargetDirection.GetSafeNormal();
				MyPawn2->AddMovementInput(Direction, 1.0f);
			}
		}
	}
}
