#include "World/Common/Player/MyAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "World/Common/Player/MyCharacter.h"
#include "Kismet/KismetMathLibrary.h"

UMyAnimInstance::UMyAnimInstance()
{
	// Inicializar variáveis
	Speed = 0.0f;
	Direction = 0.0f;
	bIsInAir = false;
	bIsAccelerating = false;
}

void UMyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Garantir que Root Motion não dirija o personagem
	SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);

	// Obter referência ao Character
	if (APawn* Pawn = TryGetPawnOwner())
	{
		MyCharacter = Cast<AMyCharacter>(Pawn);
	}
}

void UMyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn) return;

	ACharacter* Character = Cast<ACharacter>(Pawn);
	if (!Character) return;

	UCharacterMovementComponent* Move = Character->GetCharacterMovement();
	if (!Move) return;

	const FVector Vel = Character->GetVelocity();
	FVector HorizontalVel = Vel; HorizontalVel.Z = 0.f;
	Speed = HorizontalVel.Size();
	bIsInAir = Move->IsFalling();
	bIsAccelerating = Move->GetCurrentAcceleration().SizeSquared() > 1.0f;

	// Direção baseada em yaw do personagem
	if (Speed > 1.0f)
	{
		const FRotator Rot = Character->GetActorRotation();
		const FVector Fwd = FRotationMatrix(Rot).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(Rot).GetUnitAxis(EAxis::Y);
		const FVector DirN = HorizontalVel.GetSafeNormal();
		const float FwdDot = FVector::DotProduct(Fwd, DirN);
		const float RightDot = FVector::DotProduct(Right, DirN);
		Direction = FMath::RadiansToDegrees(FMath::Atan2(RightDot, FwdDot));
	}
	else
	{
		Direction = 0.f;
	}
}