#include "World/Common/Player/MyCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SphereComponent.h"
#include "World/Common/Player/PlayerHealthComponent.h"
#include "World/Common/Player/XPComponent.h"
#include "World/Common/Enemy/EnemyHealthComponent.h"
#include "Engine/Engine.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogPlayerHealth, Log, All);
DEFINE_LOG_CATEGORY_STATIC(LogXP, Log, All);

AMyCharacter::AMyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Mesh configuration
	USkeletalMeshComponent* MeshComp = GetMesh();
	MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetVisibility(true);
	MeshComp->SetHiddenInGame(false);

	// Camera Setup (Top-down)
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 600.f;
	SpringArmComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
	SpringArmComponent->SetRelativeRotation(FRotator(-45.0f, -45.0f, 0.0f));
	SpringArmComponent->bUsePawnControlRotation = false;
	SpringArmComponent->bInheritPitch = false;
	SpringArmComponent->bInheritYaw = false;
	SpringArmComponent->bInheritRoll = false;
	SpringArmComponent->SetUsingAbsoluteRotation(true);
	SpringArmComponent->bDoCollisionTest = false;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;

	// Movement
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 540.f, 0.f);
	Movement->MaxWalkSpeed = 400.0f;
	Movement->MaxAcceleration = 2048.f;
	Movement->BrakingDecelerationWalking = 4096.f;
	Movement->GroundFriction = 8.f;
	Movement->bUseSeparateBrakingFriction = true;
	Movement->BrakingFriction = 8.f;
	Movement->BrakingDecelerationFalling = 1500.0f;

	bUseControllerRotationYaw = false;

	// Stats components
	HealthComponent = CreateDefaultSubobject<UPlayerHealthComponent>(TEXT("Health"));
	XPComponent = CreateDefaultSubobject<UXPComponent>(TEXT("XP"));
	
	// Damage detection sphere
	DamageSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DamageSphere"));
	DamageSphere->SetupAttachment(RootComponent);
	DamageSphere->InitSphereRadius(120.f);
	DamageSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	DamageSphere->SetGenerateOverlapEvents(true);
	DamageSphere->OnComponentBeginOverlap.AddDynamic(this, &AMyCharacter::OnOverlapBegin);
	DamageSphere->OnComponentEndOverlap.AddDynamic(this, &AMyCharacter::OnOverlapEnd);

	// Replication
	bReplicates = true;
	SetReplicateMovement(true);
}

void AMyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (HealthComponent)
	{
		UE_LOG(LogPlayerHealth, Display, TEXT("Health component initialized"));
	}
	
	if (XPComponent)
	{
		UE_LOG(LogXP, Display, TEXT("XP component initialized"));
	}
}

void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Apply custom mesh if set
	if (CustomSkeletalMesh)
	{
		GetMesh()->SetSkeletalMesh(CustomSkeletalMesh);
	}

	// Apply custom animation if set
	if (CustomAnimInstance)
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(CustomAnimInstance);
	}
}

void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMyCharacter, CurrentHealth);
}

float AMyCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	if (HealthComponent && ActualDamage > 0.0f)
	{
		HealthComponent->ReceiveDamage(ActualDamage);
		UE_LOG(LogPlayerHealth, Warning, TEXT("Player took %.1f damage - HP: %.1f/%.1f"), 
			ActualDamage, 
			HealthComponent->GetCurrentHealth(), 
			HealthComponent->GetMaxHealth());
	}
	
	return ActualDamage;
}

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Movement bindings
	PlayerInputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AMyCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMyCharacter::LookUp);

	// Action bindings
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMyCharacter::Attack);
}

void AMyCharacter::MoveForward(float Value)
{
	if (Controller && Value != 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMyCharacter::MoveRight(float Value)
{
	if (Controller && Value != 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void AMyCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AMyCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AMyCharacter::Attack()
{
	if (bIsAttacking) return;
	
	bIsAttacking = true;
	PerformAttack();
	
	// Reset attack flag after a delay
	GetWorldTimerManager().SetTimer(AttackTimerHandle, [this]()
	{
		bIsAttacking = false;
	}, 1.0f, false);
}

void AMyCharacter::PerformAttack()
{
	UE_LOG(LogTemp, Warning, TEXT("[ATTACK] PerformAttack called - Range=%.1f, Damage=%.1f"), AttackRange, AttackDamage);
	
	// Play attack montage if available
	if (AttackMontage)
	{
		PlayAnimMontage(AttackMontage);
	}
	
	// Perform area damage attack
	TArray<FHitResult> HitResults;
	FVector Start = GetActorLocation();
	FVector End = Start + FVector(0, 0, 1); // Slight offset for better sweep detection
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.bTraceComplex = false;
	Params.bReturnPhysicalMaterial = false;
	
	// DEBUG: Draw the attack range
	DrawDebugSphere(GetWorld(), Start, AttackRange, 12, FColor::Red, false, 1.0f, 0, 2.0f);
	
	// Sphere trace to find enemies in range
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(AttackRange),
		Params
	);
	
	UE_LOG(LogTemp, Warning, TEXT("[ATTACK] Sweep result: %s, Found %d hits"), bHit ? TEXT("HIT") : TEXT("MISS"), HitResults.Num());
	
	if (bHit)
	{
		int32 EnemiesHit = 0;
		for (const FHitResult& Hit : HitResults)
		{
			if (AActor* HitActor = Hit.GetActor())
			{
				UE_LOG(LogTemp, Warning, TEXT("[ATTACK] Checking hit actor: %s (Class: %s)"), *HitActor->GetName(), *HitActor->GetClass()->GetName());
				
				// Check if it's an enemy by looking for EnemyBase class
				if (HitActor->GetClass()->GetName().Contains(TEXT("Enemy")))
				{
					// Apply damage
					FPointDamageEvent DamageEvent;
					DamageEvent.Damage = AttackDamage;
					DamageEvent.HitInfo = Hit;
					DamageEvent.ShotDirection = (HitActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
					
					float DamageApplied = HitActor->TakeDamage(AttackDamage, DamageEvent, GetController(), this);
					EnemiesHit++;
					
					UE_LOG(LogTemp, Warning, TEXT("[ATTACK] Player dealt %.1f damage to %s (Applied: %.1f)"), AttackDamage, *HitActor->GetName(), DamageApplied);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[ATTACK] Hit non-enemy: %s"), *HitActor->GetName());
				}
			}
		}
		
		UE_LOG(LogTemp, Warning, TEXT("[ATTACK] Attack complete - Hit %d enemies total"), EnemiesHit);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ATTACK] No targets found in range %.1f"), AttackRange);
	}
}

float AMyCharacter::GetCurrentHealth() const
{
	return HealthComponent ? HealthComponent->GetCurrentHealth() : 0.0f;
}

float AMyCharacter::GetMaxHealth() const
{
	return HealthComponent ? HealthComponent->GetMaxHealth() : 0.0f;
}

float AMyCharacter::GetCurrentXP() const
{
	return XPComponent ? XPComponent->GetCurrentXP() : 0.0f;
}

int32 AMyCharacter::GetCurrentLevel() const
{
	return XPComponent ? XPComponent->GetCurrentLevel() : 1;
}

void AMyCharacter::TakeDamageFromEnemy(float Damage)
{
	if (HealthComponent)
	{
		HealthComponent->ReceiveDamage(Damage);
	}
}

void AMyCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this) return;
	
	// Check if it's an enemy
	if (OtherActor->FindComponentByClass<UEnemyHealthComponent>())
	{
		UE_LOG(LogPlayerHealth, Display, TEXT("Enemy %s entered damage area"), *OtherActor->GetName());
	}
}

void AMyCharacter::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;
	
	// Check if it's an enemy
	if (OtherActor->FindComponentByClass<UEnemyHealthComponent>())
	{
		UE_LOG(LogPlayerHealth, Display, TEXT("Enemy %s left damage area"), *OtherActor->GetName());
	}
}

void AMyCharacter::OnRep_Health()
{
	// Handle health replication if needed
}
