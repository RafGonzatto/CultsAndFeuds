#include "World/Common/Player/MyCharacter.h"
#include "World/Common/Player/MyAnimInstance.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "UObject/ConstructorHelpers.h"
#include "DrawDebugHelpers.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/Skeleton.h"
#include "TimerManager.h"
#include "Engine/World.h"

AMyCharacter::AMyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Capsule Configuration
	GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);
	GetCapsuleComponent()->SetCapsuleRadius(34.0f);

	// NÃO modificar altura/rotação do Mesh via C++. Deixe o BP controlar.
	USkeletalMeshComponent* MeshComp = GetMesh();
	MeshComp->SetVisibility(true);
	MeshComp->SetHiddenInGame(false);
	// Evitar congelar pose por otimização de tick
	MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	MeshComp->bEnableUpdateRateOptimizations = false;

	// Camera Setup (Top-down)
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 600.f;
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
	SpringArm->SetRelativeRotation(FRotator(-45.0f, -45.0f, 0.0f));
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;
	SpringArm->SetUsingAbsoluteRotation(true);
	SpringArm->bDoCollisionTest = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	// Movement
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 540.f, 0.f);
	Movement->MaxWalkSpeed = 400.0f;
	// Patch 2 — frenagem “seca” (parar no lugar)
	Movement->MaxAcceleration             = 2048.f;
	Movement->BrakingDecelerationWalking  = 4096.f;
	Movement->GroundFriction              = 8.f;
	Movement->bUseSeparateBrakingFriction = true;
	Movement->BrakingFriction             = 8.f;
	Movement->BrakingDecelerationFalling = 1500.0f;

	bUseControllerRotationYaw = false;
}

void AMyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	UE_LOG(LogTemp, Warning, TEXT("[Char] PostInitializeComponents - Personagem configurado"));
}

void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	USkeletalMeshComponent* MeshComp = GetMesh();

	// Aplicar mesh custom se fornecido
	if (CustomSkeletalMesh)
	{
		MeshComp->SetSkeletalMesh(CustomSkeletalMesh);
		UE_LOG(LogTemp, Warning, TEXT("[Char] ? CustomSkeletalMesh aplicado: %s"), *CustomSkeletalMesh->GetName());
	}

	// Respeitar o AnimBP configurado no BP (ou aplicar o fornecido em AnimBPClass)
	if (AnimBPClass)
	{
		MeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		MeshComp->SetAnimInstanceClass(AnimBPClass);
		UE_LOG(LogTemp, Warning, TEXT("[Char] ? AnimBPClass aplicado: %s"), *AnimBPClass->GetName());
	}

	// Nunca forçar offsets do mesh em runtime
	// MeshComp->SetRelativeLocation/Rotation removidos

	// Posicionamento no chão (sem mexer no mesh offset)
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		FHitResult Hit;
		FVector Start = GetActorLocation() + FVector(0,0,50);
		FVector End = Start - FVector(0,0,500);
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
		{
			SetActorLocation(Hit.ImpactPoint + FVector(0, 0, HalfHeight + 5.f));
		}
	}

	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->SetMovementMode(MOVE_Walking);
	}
}

// ============================================================================
// SISTEMA DE TAUNTS ÚNICO E DEFINITIVO - SEM FALLBACKS
// ============================================================================

bool AMyCharacter::HasActiveTaunt() const
{
	return CurrentTauntMontage != nullptr && bTauntWasSuccessfullyStarted;
}

void AMyCharacter::OnTauntMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == CurrentTauntMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Char] ? Taunt '%s' finalizado (interrupted=%s)"), 
			*Montage->GetName(), 
			bInterrupted ? TEXT("true") : TEXT("false"));
		
		// Limpar estado
		CurrentTauntMontage = nullptr;
		bTauntWasSuccessfullyStarted = false;
		TauntInterruptGraceEndTime = 0.f;
	}
}

void AMyCharacter::InterruptTaunt(float BlendOutTime)
{
	// Só interromper se há taunt realmente ativo
	if (!HasActiveTaunt()) return;

	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (!Anim) return;

	// Verificar grace period
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime < TauntInterruptGraceEndTime)
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("[Char] Taunt em grace period, não interrompendo"));
		return;
	}

	// Interromper montage
	if (Anim->Montage_IsPlaying(CurrentTauntMontage))
	{
		Anim->Montage_Stop(BlendOutTime, CurrentTauntMontage);
		UE_LOG(LogTemp, Warning, TEXT("[Char] ? Taunt '%s' interrompido"), *CurrentTauntMontage->GetName());
	}
}

void AMyCharacter::PlayAnim1()
{
	UE_LOG(LogTemp, Warning, TEXT("[Char] ?? TENTANDO TOCAR ANIM1..."));

	// EXECUÇÃO COM DIAGNÓSTICO COMPLETO
	if (!AnimMontage1)
	{
		UE_LOG(LogTemp, Error, TEXT("[Char] ? AnimMontage1 NÃO CONFIGURADO"));
		return;
	}

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		UE_LOG(LogTemp, Error, TEXT("[Char] ? MeshComp é NULL"));
		return;
	}

	if (!MeshComp->GetSkeletalMeshAsset())
	{
		UE_LOG(LogTemp, Error, TEXT("[Char] ? SkeletalMeshAsset é NULL"));
		return;
	}

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("[Char] ? AnimInstance é NULL - TENTANDO RECRIAR"));
		
		// *** TENTATIVA DE RECUPERAÇÃO ***
		MeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		MeshComp->SetAnimInstanceClass(UMyAnimInstance::StaticClass());
		
		// Usar sintaxe correta para timer com delegate
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindUFunction(this, FName("PlayAnim1"));
		
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.1f, false);
		return;
	}

	// VERIFICAR TIPO DO ANIMINSTANCE
	UE_LOG(LogTemp, Warning, TEXT("[Char] AnimInstance tipo: %s"), *AnimInstance->GetClass()->GetName());

	// Interromper taunt anterior
	InterruptTaunt(0.1f);

	// TOCAR MONTAGE
	const float PlayLength = AnimInstance->Montage_Play(AnimMontage1, 1.0f);
	
	UE_LOG(LogTemp, Warning, TEXT("[Char] Montage_Play Anim1 resultado: %.2f"), PlayLength);

	if (PlayLength > 0.f)
	{
		// *** CRÍTICO: CONFIGURAR ROOT MOTION APÓS TOCAR O MONTAGE ***
		AnimInstance->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
		
		// SUCESSO
		CurrentTauntMontage = AnimMontage1;
		bTauntWasSuccessfullyStarted = true;
		TauntInterruptGraceEndTime = GetWorld()->GetTimeSeconds() + 0.4f;
		
		// Configurar callback
		AnimInstance->OnMontageEnded.RemoveAll(this);
		AnimInstance->OnMontageEnded.AddDynamic(this, &AMyCharacter::OnTauntMontageEnded);
		
		UE_LOG(LogTemp, Warning, TEXT("[Char] ?????? ANIM1 TOCANDO COM SUCESSO! ??????"));
		UE_LOG(LogTemp, Warning, TEXT("[Char] ? Root Motion configurado para IGNORE após montage"));
	}
	else
	{
		// DIAGNÓSTICO DE FALHA
		UE_LOG(LogTemp, Error, TEXT("[Char] ??? FALHA AO TOCAR ANIM1 ???"));
		UE_LOG(LogTemp, Error, TEXT("[Char] Montage: %s"), AnimMontage1 ? *AnimMontage1->GetName() : TEXT("NULL"));
		UE_LOG(LogTemp, Error, TEXT("[Char] AnimInstance: %s"), *AnimInstance->GetClass()->GetName());
		UE_LOG(LogTemp, Error, TEXT("[Char] SkeletalMesh: %s"), *MeshComp->GetSkeletalMeshAsset()->GetName());
		
		// VERIFICAR COMPATIBILIDADE DE SKELETON
		if (AnimMontage1->GetSkeleton() && MeshComp->GetSkeletalMeshAsset()->GetSkeleton())
		{
			bool bCompatible = (AnimMontage1->GetSkeleton() == MeshComp->GetSkeletalMeshAsset()->GetSkeleton());
			UE_LOG(LogTemp, Error, TEXT("[Char] Skeleton compatível: %s"), bCompatible ? TEXT("SIM") : TEXT("NÃO"));
			
			if (!bCompatible)
			{
				UE_LOG(LogTemp, Error, TEXT("[Char] Montage Skeleton: %s"), *AnimMontage1->GetSkeleton()->GetName());
				UE_LOG(LogTemp, Error, TEXT("[Char] Mesh Skeleton: %s"), *MeshComp->GetSkeletalMeshAsset()->GetSkeleton()->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[Char] Um dos skeletons é NULL!"));
		}
		
		// VERIFICAR SE ANIMINSTANCE ESTÁ INICIALIZADO
		if (AnimInstance->GetWorld())
		{
			UE_LOG(LogTemp, Warning, TEXT("[Char] AnimInstance tem World válido"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[Char] AnimInstance SEM World!"));
		}
	}
}

void AMyCharacter::PlayAnim2()
{
	UE_LOG(LogTemp, Warning, TEXT("[Char] ?? TENTANDO TOCAR ANIM2..."));

	// EXECUÇÃO COM DIAGNÓSTICO COMPLETO
	if (!AnimMontage2)
	{
		UE_LOG(LogTemp, Error, TEXT("[Char] ? AnimMontage2 NÃO CONFIGURADO"));
		return;
	}

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		UE_LOG(LogTemp, Error, TEXT("[Char] ? MeshComp é NULL"));
		return;
	}

	if (!MeshComp->GetSkeletalMeshAsset())
	{
		UE_LOG(LogTemp, Error, TEXT("[Char] ? SkeletalMeshAsset é NULL"));
		return;
	}

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("[Char] ? AnimInstance é NULL - TENTANDO RECRIAR"));
		
		// *** TENTATIVA DE RECUPERAÇÃO ***
		MeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		MeshComp->SetAnimInstanceClass(UMyAnimInstance::StaticClass());
		
		// Usar sintaxe correta para timer com delegate
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindUFunction(this, FName("PlayAnim2"));
		
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.1f, false);
		return;
	}

	// VERIFICAR TIPO DO ANIMINSTANCE
	UE_LOG(LogTemp, Warning, TEXT("[Char] AnimInstance tipo: %s"), *AnimInstance->GetClass()->GetName());

	// Interromper taunt anterior
	InterruptTaunt(0.1f);

	// TOCAR MONTAGE
	const float PlayLength = AnimInstance->Montage_Play(AnimMontage2, 1.0f);
	
	UE_LOG(LogTemp, Warning, TEXT("[Char] Montage_Play Anim2 resultado: %.2f"), PlayLength);

	if (PlayLength > 0.f)
	{
		// *** CRÍTICO: CONFIGURAR ROOT MOTION APÓS TOCAR O MONTAGE ***
		AnimInstance->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
		
		// SUCESSO
		CurrentTauntMontage = AnimMontage2;
		bTauntWasSuccessfullyStarted = true;
		TauntInterruptGraceEndTime = GetWorld()->GetTimeSeconds() + 0.4f;
		
		// Configurar callback
		AnimInstance->OnMontageEnded.RemoveAll(this);
		AnimInstance->OnMontageEnded.AddDynamic(this, &AMyCharacter::OnTauntMontageEnded);
		
		UE_LOG(LogTemp, Warning, TEXT("[Char] ?????? ANIM2 TOCANDO COM SUCESSO! ??????"));
		UE_LOG(LogTemp, Warning, TEXT("[Char] ? Root Motion configurado para IGNORE após montage"));
	}
	else
	{
		// DIAGNÓSTICO DE FALHA
		UE_LOG(LogTemp, Error, TEXT("[Char] ??? FALHA AO TOCAR ANIM2 ???"));
		UE_LOG(LogTemp, Error, TEXT("[Char] Montage: %s"), AnimMontage2 ? *AnimMontage2->GetName() : TEXT("NULL"));
		UE_LOG(LogTemp, Error, TEXT("[Char] AnimInstance: %s"), *AnimInstance->GetClass()->GetName());
		UE_LOG(LogTemp, Error, TEXT("[Char] SkeletalMesh: %s"), *MeshComp->GetSkeletalMeshAsset()->GetName());
		
		// VERIFICAR COMPATIBILIDADE DE SKELETON
		if (AnimMontage2->GetSkeleton() && MeshComp->GetSkeletalMeshAsset()->GetSkeleton())
		{
			bool bCompatible = (AnimMontage2->GetSkeleton() == MeshComp->GetSkeletalMeshAsset()->GetSkeleton());
			UE_LOG(LogTemp, Error, TEXT("[Char] Skeleton compatível: %s"), bCompatible ? TEXT("SIM") : TEXT("NÃO"));
			
			if (!bCompatible)
			{
				UE_LOG(LogTemp, Error, TEXT("[Char] Montage Skeleton: %s"), *AnimMontage2->GetSkeleton()->GetName());
				UE_LOG(LogTemp, Error, TEXT("[Char] Mesh Skeleton: %s"), *MeshComp->GetSkeletalMeshAsset()->GetSkeleton()->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[Char] Um dos skeletons é NULL!"));
		}
	}
}

void AMyCharacter::CharDump()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	USkeletalMesh* SkeletalMeshAsset = MeshComp ? MeshComp->GetSkeletalMeshAsset() : nullptr;
	UClass* AnimClass = MeshComp ? MeshComp->GetAnimClass() : nullptr;
	
	UE_LOG(LogTemp, Warning, TEXT("=== CHARACTER DEBUG DUMP ==="));
	UE_LOG(LogTemp, Warning, TEXT("Mesh: %s"), SkeletalMeshAsset ? *SkeletalMeshAsset->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("AnimClass: %s"), AnimClass ? *AnimClass->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("HasActiveTaunt: %s"), HasActiveTaunt() ? TEXT("TRUE") : TEXT("FALSE"));
	UE_LOG(LogTemp, Warning, TEXT("CurrentTaunt: %s"), CurrentTauntMontage ? *CurrentTauntMontage->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("Anim1: %s"), AnimMontage1 ? *AnimMontage1->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("Anim2: %s"), AnimMontage2 ? *AnimMontage2->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("========================"));
}

void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// DEBUG: Rastrear posições para detectar problemas
	DebugPositionTracking();
}

void AMyCharacter::DebugPositionTracking()
{
	FVector CurrentPosition = GetActorLocation();
	
	if (!LastTickPosition.IsZero())
	{
		float DistanceMoved = FVector::Dist(CurrentPosition, LastTickPosition);
		
		// Detectar saltos de posição suspeitos
		if (DistanceMoved > 200.0f && DistanceMoved < 5000.0f) // Evitar falsos positivos
		{
			PositionJumpCount++;
			UE_LOG(LogTemp, Error, TEXT("?? [CHARACTER] Posição saltou %.1f unidades! (#%d)"), 
				DistanceMoved, PositionJumpCount);
			UE_LOG(LogTemp, Error, TEXT("?? De: %s | Para: %s"), 
				*LastTickPosition.ToString(), *CurrentPosition.ToString());
				
			// Verificar se há discrepância entre componentes
			ValidateComponentAlignment();
		}
	}
	
	LastTickPosition = CurrentPosition;
	
	// Debug periódico
	DebugUpdateTimer += GetWorld()->GetDeltaSeconds();
	if (DebugUpdateTimer >= 2.0f) // A cada 2 segundos
	{
		ValidateComponentAlignment();
		DebugUpdateTimer = 0.f;
	}
}

void AMyCharacter::ValidateComponentAlignment()
{
	FVector ActorLoc = GetActorLocation();
	FVector MeshLoc = GetMesh()->GetComponentLocation();
	FVector CapsuleLoc = GetCapsuleComponent()->GetComponentLocation();
	
	float MeshDiscrepancy = FVector::Dist(ActorLoc, MeshLoc);
	float CapsuleDiscrepancy = FVector::Dist(ActorLoc, CapsuleLoc);
	
	if (MeshDiscrepancy > 50.0f || CapsuleDiscrepancy > 50.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("?? [ALIGNMENT] Actor: %s"), *ActorLoc.ToString());
		UE_LOG(LogTemp, Error, TEXT("?? [ALIGNMENT] Mesh: %s (Disc: %.1f)"), *MeshLoc.ToString(), MeshDiscrepancy);
		UE_LOG(LogTemp, Error, TEXT("?? [ALIGNMENT] Capsule: %s (Disc: %.1f)"), *CapsuleLoc.ToString(), CapsuleDiscrepancy);
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			const FString MontageName = AnimInstance->GetCurrentActiveMontage() ? AnimInstance->GetCurrentActiveMontage()->GetName() : TEXT("None");
			UE_LOG(LogTemp, Error, TEXT("?? [ALIGNMENT] Montage: %s"), *MontageName);
		}
	}
}

// ============================================================================
// COMANDOS DE DEBUG EXECUTÁVEIS
// ============================================================================

void AMyCharacter::DebugRootMotion()
{
	UE_LOG(LogTemp, Warning, TEXT("=== ROOT MOTION DEBUG ==="));
	
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		const FString MontageName = AnimInstance->GetCurrentActiveMontage() ? 
			AnimInstance->GetCurrentActiveMontage()->GetName() : 
			TEXT("None");
		UE_LOG(LogTemp, Warning, TEXT("Active Montage: %s"), *MontageName);
		
		if (AnimInstance->GetCurrentActiveMontage())
		{
			UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();
			UE_LOG(LogTemp, Warning, TEXT("Montage Position: %.3f / %.3f"), 
				AnimInstance->Montage_GetPosition(CurrentMontage), 
				CurrentMontage->GetPlayLength());
		}
		
		// LOG SKELETON COMPATIBILITY (sem deprecated IsCompatible)
		if (GetMesh()->GetSkeletalMeshAsset())
		{
			USkeleton* MeshSkeleton = GetMesh()->GetSkeletalMeshAsset()->GetSkeleton();
			UE_LOG(LogTemp, Warning, TEXT("Mesh Skeleton: %s"), MeshSkeleton ? *MeshSkeleton->GetName() : TEXT("NULL"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("? AnimInstance é NULL!"));
	}
}

void AMyCharacter::ForceIgnoreRootMotion()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
		UE_LOG(LogTemp, Warning, TEXT("? [FORCE] Root Motion configurado para IGNORE"));
		
		// PARAR QUALQUER MONTAGE ATIVO
		if (UAnimMontage* ActiveMontage = AnimInstance->GetCurrentActiveMontage())
		{
			AnimInstance->Montage_Stop(0.1f, ActiveMontage);
			UE_LOG(LogTemp, Warning, TEXT("? [FORCE] Montage parado: %s"), *ActiveMontage->GetName());
		}
		
		// RESETAR TAUNTS
		CurrentTauntMontage = nullptr;
		bTauntWasSuccessfullyStarted = false;
		TauntInterruptGraceEndTime = 0.f;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("? [FORCE] AnimInstance é NULL"));
	}
}

void AMyCharacter::ValidateSetup()
{
	UE_LOG(LogTemp, Warning, TEXT("=== VALIDAÇÃO COMPLETA DO SETUP ==="));
	
	// VALIDAR SKELETAL MESH
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp && MeshComp->GetSkeletalMeshAsset())
	{
		UE_LOG(LogTemp, Warning, TEXT("? SkeletalMesh: %s"), *MeshComp->GetSkeletalMeshAsset()->GetName());
		
		if (USkeleton* MeshSkeleton = MeshComp->GetSkeletalMeshAsset()->GetSkeleton())
		{
			UE_LOG(LogTemp, Warning, TEXT("? Skeleton: %s"), *MeshSkeleton->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("? Skeleton é NULL"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("? SkeletalMeshAsset é NULL"));
	}
	
	// VALIDAR ANIM INSTANCE COM DETALHES
	if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
	{
		UE_LOG(LogTemp, Warning, TEXT("? AnimInstance: %s"), *AnimInstance->GetClass()->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("? AnimInstance é NULL"));
	}
	
	// VALIDAR MONTAGES
	if (AnimMontage1)
	{
		UE_LOG(LogTemp, Warning, TEXT("? AnimMontage1: %s | Length: %.2f"), 
			*AnimMontage1->GetName(), AnimMontage1->GetPlayLength());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("? AnimMontage1 é NULL"));
	}
	
	if (AnimMontage2)
	{
		UE_LOG(LogTemp, Warning, TEXT("? AnimMontage2: %s | Length: %.2f"), 
			*AnimMontage2->GetName(), AnimMontage2->GetPlayLength());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("? AnimMontage2 é NULL"));
	}
	
	// VALIDAR MOVEMENT COMPONENT
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		UE_LOG(LogTemp, Warning, TEXT("? CharacterMovement: MaxWalkSpeed=%.1f Mode=%d"), 
			MovementComp->MaxWalkSpeed, (int32)MovementComp->MovementMode);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("? CharacterMovementComponent é NULL"));
	}
	
	// VALIDAR POSIÇÕES
	ValidateComponentAlignment();
	
	UE_LOG(LogTemp, Warning, TEXT("=== FIM DA VALIDAÇÃO ==="));
}
