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
#include "World/Common/Player/PlayerHealthComponent.h"
#include "World/Common/Player/XPComponent.h"
#include "World/Common/Enemy/EnemyHealthComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Swarm/SwarmSubsystem.h"
#include "Net/UnrealNetwork.h"

// Categorias de log espec�ficas para cada sistema
DEFINE_LOG_CATEGORY_STATIC(LogPlayerHealth, Log, All);
DEFINE_LOG_CATEGORY_STATIC(LogXP, Log, All);
DEFINE_LOG_CATEGORY_STATIC(LogAttack, Log, All);

AMyCharacter::AMyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Capsule Configuration
	GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);
	GetCapsuleComponent()->SetCapsuleRadius(34.0f);

	USkeletalMeshComponent* MeshComp = GetMesh();
	MeshComp->SetVisibility(true);
	MeshComp->SetHiddenInGame(false);
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
	Movement->MaxAcceleration             = 2048.f;
	Movement->BrakingDecelerationWalking  = 4096.f;
	Movement->GroundFriction              = 8.f;
	Movement->bUseSeparateBrakingFriction = true;
	Movement->BrakingFriction             = 8.f;
	Movement->BrakingDecelerationFalling  = 1500.0f;

	bUseControllerRotationYaw = false;

	// Stats - Componentes RPG - IMPORTANTE: CreateDefaultSubobject e SetupAttachment
	Health = CreateDefaultSubobject<UPlayerHealthComponent>(TEXT("Health"));
	XP = CreateDefaultSubobject<UXPComponent>(TEXT("XP"));
	
	DamageSense = CreateDefaultSubobject<USphereComponent>(TEXT("DamageSense"));
	DamageSense->SetupAttachment(RootComponent);
	DamageSense->InitSphereRadius(120.f);
	DamageSense->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	DamageSense->SetGenerateOverlapEvents(true);
	DamageSense->OnComponentBeginOverlap.AddDynamic(this, &AMyCharacter::OnSenseBegin);
	DamageSense->OnComponentEndOverlap.AddDynamic(this, &AMyCharacter::OnSenseEnd);
}

void AMyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// Add any custom replicated properties here if needed
	// DOREPLIFETIME(AMyCharacter, SomeProperty);
}

void AMyCharacter::OnSenseBegin(UPrimitiveComponent*, AActor* Other, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (!Other || Other == this) return;
	if (Other->FindComponentByClass<UEnemyHealthComponent>())
	{
		ContactingEnemies.Add(Other);
		UE_LOG(LogPlayerHealth, Display, TEXT("Enemy %s entered damage sense"), *Other->GetName());
	}
}

void AMyCharacter::OnSenseEnd(UPrimitiveComponent*, AActor* Other, UPrimitiveComponent*, int32)
{
	if (Other) {
		ContactingEnemies.Remove(Other);
		UE_LOG(LogPlayerHealth, Display, TEXT("Enemy %s left damage sense"), *Other->GetName());
	}
}

void AMyCharacter::PerformAreaAttack(float Radius, float Damage)
{
	UWorld* W = GetWorld(); if (!W) return;
	TArray<AActor*> Ignored; Ignored.Add(this);
	TArray<AActor*> Hits;

#if !(UE_BUILD_SHIPPING)
	DrawDebugSphere(W, GetActorLocation(), Radius, 24, FColor::Cyan, false, 0.3f, 0, 2.f);
#endif

	// Inimigos tradicionais
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjTypes;
	ObjTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	ObjTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	const bool bAnyTraditional = UKismetSystemLibrary::SphereOverlapActors(
		this,
		GetActorLocation(),
		Radius,
		ObjTypes,
		nullptr,
		Ignored,
		Hits
	);
	int32 TraditionalHits = 0;
	if (bAnyTraditional)
	{
		for (AActor* A : Hits)
		{
			if (UEnemyHealthComponent* E = A->FindComponentByClass<UEnemyHealthComponent>())
			{
				E->ReceiveDamage(Damage);
				TraditionalHits++;
#if !(UE_BUILD_SHIPPING)
				DrawDebugLine(W, GetActorLocation(), A->GetActorLocation(), FColor::Red, false, 0.5f, 0, 3.0f);
#endif
			}
		}
	}

	// Enxame
	int32 SwarmHits = 0;
	if (USwarmSubsystem* SS = W->GetSubsystem<USwarmSubsystem>())
	{
		SwarmHits = SS->ApplyRadialDamage(GetActorLocation(), Radius, Damage);
	}
	else
	{
		UE_LOG(LogAttack, Verbose, TEXT("PerformAreaAttack: SwarmSubsystem nao encontrado"));
	}

	UE_LOG(LogAttack, Display, TEXT("AreaAttack Result -> Tradicionais=%d | Swarm=%d | R=%.0f Dmg=%.0f"), TraditionalHits, SwarmHits, Radius, Damage);
}

void AMyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	UE_LOG(LogTemp, Warning, TEXT("[Char] PostInitializeComponents - Personagem configurado"));
	
	// Verificar se os componentes foram criados corretamente
	if (!Health || !XP)
	{
		UE_LOG(LogTemp, Error, TEXT("[Char] PostInitializeComponents - Health ou XP n�o est�o inicializados!"));
	}
	else
	{
		UE_LOG(LogPlayerHealth, Display, TEXT("Health component inicializado"));
		UE_LOG(LogPlayerHealth, Display, TEXT("Max=%.1f Current=%.1f"), Health->GetMaxHealth(), Health->GetCurrentHealth());
		UE_LOG(LogXP, Display, TEXT("XP component inicializado"));
		UE_LOG(LogXP, Display, TEXT("Level=%d XP=%d/%d"), XP->GetCurrentLevel(), XP->GetCurrentXP(), XP->GetXPToNextLevel());
	}
}

void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	USkeletalMeshComponent* MeshComp = GetMesh();

	if (CustomSkeletalMesh)
	{
		MeshComp->SetSkeletalMesh(CustomSkeletalMesh);
		UE_LOG(LogTemp, Warning, TEXT("[Char] CustomSkeletalMesh aplicado: %s"), *CustomSkeletalMesh->GetName());
	}

	if (AnimBPClass)
	{
		MeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		MeshComp->SetAnimInstanceClass(AnimBPClass);
		UE_LOG(LogTemp, Warning, TEXT("[Char] AnimBPClass aplicado: %s"), *AnimBPClass->GetName());
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		FHitResult Hit;
		FVector Start = GetActorLocation() + FVector(0, 0, 50);
		FVector End = Start - FVector(0, 0, 500);
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
		{
			SetActorLocation(Hit.ImpactPoint + FVector(0, 0, HalfHeight + 5.f));
		}
	}

	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->SetMovementMode(MOVE_Walking);
	}

	// Inicializa��o dos componentes de Health e XP
	if (Health)
	{
		// Garantir inicializa��o adequada
		Health->OnDeath.AddDynamic(this, &AMyCharacter::OnPlayerDeath);
		UE_LOG(LogPlayerHealth, Display, TEXT("Health inicializado"));
		UE_LOG(LogPlayerHealth, Display, TEXT("HP=%.1f/%.1f"), 
			Health->GetCurrentHealth(), Health->GetMaxHealth());
	}
	else
	{
		UE_LOG(LogPlayerHealth, Error, TEXT("Health component n�o foi inicializado!"));
	}
	
	if (XP)
	{
		UE_LOG(LogXP, Display, TEXT("XP inicializado"));
		UE_LOG(LogXP, Display, TEXT("Level=%d XP=%d/%d"), 
			XP->GetCurrentLevel(), XP->GetCurrentXP(), XP->GetXPToNextLevel());
	}
	else
	{
		UE_LOG(LogXP, Error, TEXT("XP component n�o foi inicializado!"));
	}
}

void AMyCharacter::OnPlayerDeath()
{
	UE_LOG(LogPlayerHealth, Warning, TEXT("Player morreu (OnPlayerDeath handler)"));
}

bool AMyCharacter::HasActiveTaunt() const
{
	return CurrentTauntMontage != nullptr && bTauntWasSuccessfullyStarted;
}

void AMyCharacter::OnTauntMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == CurrentTauntMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Char] Taunt '%s' finalizado (interrupted=%s)"), *Montage->GetName(), bInterrupted ? TEXT("true") : TEXT("false"));
		CurrentTauntMontage = nullptr;
		bTauntWasSuccessfullyStarted = false;
		TauntInterruptGraceEndTime = 0.f;
	}
}

void AMyCharacter::InterruptTaunt(float BlendOutTime)
{
	if (!HasActiveTaunt()) return;
	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (!Anim) return;
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime < TauntInterruptGraceEndTime) return;
	if (Anim->Montage_IsPlaying(CurrentTauntMontage))
	{
		Anim->Montage_Stop(BlendOutTime, CurrentTauntMontage);
		UE_LOG(LogTemp, Warning, TEXT("[Char] Taunt '%s' interrompido"), *CurrentTauntMontage->GetName());
	}
}

void AMyCharacter::PlayAnim1()
{
	if (!AnimMontage1) { UE_LOG(LogTemp, Error, TEXT("[Char] AnimMontage1 n?o configurado")); return; }
	USkeletalMeshComponent* MeshComp = GetMesh(); if (!MeshComp || !MeshComp->GetSkeletalMeshAsset()) return;
	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance)
	{
		MeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		MeshComp->SetAnimInstanceClass(UMyAnimInstance::StaticClass());
		FTimerHandle Th; FTimerDelegate D; D.BindUFunction(this, FName("PlayAnim1"));
		GetWorld()->GetTimerManager().SetTimer(Th, D, 0.1f, false);
		return;
	}
	InterruptTaunt(0.1f);
	const float PlayLength = AnimInstance->Montage_Play(AnimMontage1, 1.0f);
	if (PlayLength > 0.f)
	{
		AnimInstance->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
		CurrentTauntMontage = AnimMontage1;
		bTauntWasSuccessfullyStarted = true;
		TauntInterruptGraceEndTime = GetWorld()->GetTimeSeconds() + 0.4f;
		AnimInstance->OnMontageEnded.RemoveAll(this);
		AnimInstance->OnMontageEnded.AddDynamic(this, &AMyCharacter::OnTauntMontageEnded);
	}
}

void AMyCharacter::PlayAnim2()
{
	if (!AnimMontage2) { UE_LOG(LogTemp, Error, TEXT("[Char] AnimMontage2 n?o configurado")); return; }
	USkeletalMeshComponent* MeshComp = GetMesh(); if (!MeshComp || !MeshComp->GetSkeletalMeshAsset()) return;
	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance)
	{
		MeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		MeshComp->SetAnimInstanceClass(UMyAnimInstance::StaticClass());
		FTimerHandle Th; FTimerDelegate D; D.BindUFunction(this, FName("PlayAnim2"));
		GetWorld()->GetTimerManager().SetTimer(Th, D, 0.1f, false);
		return;
	}
	InterruptTaunt(0.1f);
	const float PlayLength = AnimInstance->Montage_Play(AnimMontage2, 1.0f);
	if (PlayLength > 0.f)
	{
		AnimInstance->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
		CurrentTauntMontage = AnimMontage2;
		bTauntWasSuccessfullyStarted = true;
		TauntInterruptGraceEndTime = GetWorld()->GetTimeSeconds() + 0.4f;
		AnimInstance->OnMontageEnded.RemoveAll(this);
		AnimInstance->OnMontageEnded.AddDynamic(this, &AMyCharacter::OnTauntMontageEnded);
	}
}

void AMyCharacter::CharDump()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	USkeletalMesh* SkeletalMeshAsset = MeshComp ? MeshComp->GetSkeletalMeshAsset() : nullptr;
	UClass* AnimClass = MeshComp ? MeshComp->GetAnimClass() : nullptr;
	UE_LOG(LogTemp, Warning, TEXT("=== CHARACTER DEBUG DUMP ==="));
	UE_LOG(LogTemp, Warning, TEXT("Mesh=%s"), SkeletalMeshAsset ? *SkeletalMeshAsset->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("AnimClass=%s"), AnimClass ? *AnimClass->GetName() : TEXT("NULL"));
	if (Health) UE_LOG(LogTemp, Warning, TEXT("Health=%.1f/%.1f"), Health->GetCurrentHealth(), Health->GetMaxHealth());
	if (XP) UE_LOG(LogTemp, Warning, TEXT("XP=Level %d %d/%d"), XP->GetCurrentLevel(), XP->GetCurrentXP(), XP->GetXPToNextLevel());
	UE_LOG(LogTemp, Warning, TEXT("========================"));
}

void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Health && ContactingEnemies.Num() > 0)
	{
		const float TotalDmg = ContactDPS * ContactingEnemies.Num() * DeltaTime;
		Health->ReceiveDamage(TotalDmg);
	}
}

void AMyCharacter::DebugPositionTracking()
{
	FVector CurrentPosition = GetActorLocation();
	if (!LastTickPosition.IsZero())
	{
		float DistanceMoved = FVector::Dist(CurrentPosition, LastTickPosition);
		if (DistanceMoved > 200.0f && DistanceMoved < 5000.0f)
		{
			++PositionJumpCount;
			ValidateComponentAlignment();
		}
	}
	LastTickPosition = CurrentPosition;
	DebugUpdateTimer = GetWorld()->GetDeltaSeconds();
	if (DebugUpdateTimer >= 2.0f)
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
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			const FString MontageName = AnimInstance->GetCurrentActiveMontage() ? AnimInstance->GetCurrentActiveMontage()->GetName() : TEXT("None");
			// UE_LOG(LogTemp, VeryVerbose, TEXT("Alignment check montage: %s"), *MontageName); // opcional
		}
	}
}

void AMyCharacter::DebugRootMotion()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (AnimInstance->GetCurrentActiveMontage())
		{
			UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();
			AnimInstance->Montage_GetPosition(CurrentMontage);
		}
		if (GetMesh()->GetSkeletalMeshAsset())
		{
			GetMesh()->GetSkeletalMeshAsset()->GetSkeleton();
		}
	}
}

void AMyCharacter::ForceIgnoreRootMotion()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
		if (UAnimMontage* ActiveMontage = AnimInstance->GetCurrentActiveMontage())
		{
			AnimInstance->Montage_Stop(0.1f, ActiveMontage);
		}
		CurrentTauntMontage = nullptr;
		bTauntWasSuccessfullyStarted = false;
		TauntInterruptGraceEndTime = 0.f;
	}
}

void AMyCharacter::ValidateSetup()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp && MeshComp->GetSkeletalMeshAsset())
	{
		if (USkeleton* MeshSkeleton = MeshComp->GetSkeletalMeshAsset()->GetSkeleton())
		{
		}
	}
	if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
	{
	}
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		(void)MovementComp->MaxWalkSpeed;
	}
	ValidateComponentAlignment();
}
