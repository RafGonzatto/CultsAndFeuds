#include "World/Common/Player/MyPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "GameFramework/Character.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerInput.h"
#include "InputCoreTypes.h"

AMyPlayerController::AMyPlayerController()
{
	bShowMouseCursor = false;
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP) return;

	UEnhancedInputLocalPlayerSubsystem* Subsys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsys) return;

	// Criar Mapping Context e Ações em runtime (sem assets)
	Mapping = NewObject<UInputMappingContext>(this, TEXT("IMC_Runtime"));
	MoveForwardAction = NewObject<UInputAction>(this, TEXT("IA_MoveForward"));
	MoveRightAction = NewObject<UInputAction>(this, TEXT("IA_MoveRight"));
	LookYawAction = NewObject<UInputAction>(this, TEXT("IA_LookYaw"));

	// Tipo Axis1D
	MoveForwardAction->ValueType = EInputActionValueType::Axis1D;
	MoveRightAction->ValueType = EInputActionValueType::Axis1D;
	LookYawAction->ValueType = EInputActionValueType::Axis1D;

	// Mapear teclas
	// Frente/Trás
	{
		FEnhancedActionKeyMapping& W = Mapping->MapKey(MoveForwardAction, EKeys::W);
		W.Scale = 1.f;
		FEnhancedActionKeyMapping& S = Mapping->MapKey(MoveForwardAction, EKeys::S);
		S.Scale = -1.f;
	}
	// Direita/Esquerda
	{
		FEnhancedActionKeyMapping& D = Mapping->MapKey(MoveRightAction, EKeys::D);
		D.Scale = 1.f;
		FEnhancedActionKeyMapping& A = Mapping->MapKey(MoveRightAction, EKeys::A);
		A.Scale = -1.f;
	}
	// Olhar (Yaw)
	{
		FEnhancedActionKeyMapping& MX = Mapping->MapKey(LookYawAction, EKeys::MouseX);
		MX.Scale = 1.f;
	}

	// Adicionar o contexto
	Subsys->AddMappingContext(Mapping, /*Priority=*/0);
}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AMyPlayerController::OnMoveForward);
		EIC->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AMyPlayerController::OnMoveRight);
		EIC->BindAction(LookYawAction, ETriggerEvent::Triggered, this, &AMyPlayerController::OnLookYaw);
	}
}

void AMyPlayerController::OnMoveForward(const FInputActionValue& Value)
{
	if (APawn* P = GetPawn())
	{
		const float Axis = Value.Get<float>();
		const FRotator YawRot(0.f, GetControlRotation().Yaw, 0.f);
		const FVector  Dir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
		P->AddMovementInput(Dir, Axis);
	}
}

void AMyPlayerController::OnMoveRight(const FInputActionValue& Value)
{
	if (APawn* P = GetPawn())
	{
		const float Axis = Value.Get<float>();
		const FRotator YawRot(0.f, GetControlRotation().Yaw, 0.f);
		const FVector  Dir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
		P->AddMovementInput(Dir, Axis);
	}
}

void AMyPlayerController::OnLookYaw(const FInputActionValue& Value)
{
	AddYawInput(Value.Get<float>());
}
