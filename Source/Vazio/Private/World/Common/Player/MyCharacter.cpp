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
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Engine/EngineTypes.h"
#include "Gameplay/Upgrades/UpgradeSystem.h"
#include "UI/LevelUp/SLevelUpModal.h"
#include "Framework/Application/SlateApplication.h"

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

	SpawnDefaultWeapons();
	
	// Connect XP Level Up delegate
	if (XPComponent)
	{
		XPComponent->OnLevelChanged.AddDynamic(this, &AMyCharacter::OnPlayerLevelUp);
		UE_LOG(LogXP, Log, TEXT("[MyCharacter] Connected to XPComponent OnLevelChanged delegate"));
	}
}

void AMyCharacter::SpawnDefaultWeapons()
{
	// Weapon system removed - was part of old Swarm system
	// TODO: Implement new generic weapon system if needed
}

bool AMyCharacter::IsInSwarmBattleLevel() const
{
	if (const UWorld* World = GetWorld())
	{
		return World->GetMapName().Contains(TEXT("Battle_Main"));
	}

	return false;
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
	
	UE_LOG(LogPlayerHealth, Warning, TEXT("[PLAYER-OVERLAP] %s (%s) overlapped with %s (%s)"), 
        *GetName(),
        OverlappedComp ? *OverlappedComp->GetName() : TEXT("NULL"), 
        *OtherActor->GetName(),
        OtherComp ? *OtherComp->GetName() : TEXT("NULL"));
	
	// Check if it's an enemy
	if (OtherActor->FindComponentByClass<UEnemyHealthComponent>())
	{
		UE_LOG(LogPlayerHealth, Warning, TEXT("[PLAYER-ENEMY-DETECTED] Enemy %s entered damage area"), *OtherActor->GetName());
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

// ============================================================================
// LEVEL UP SYSTEM
// ============================================================================

void AMyCharacter::OnPlayerLevelUp(int32 NewLevel)
{
	UE_LOG(LogXP, Warning, TEXT("[MyCharacter] ⭐ LEVEL UP! New Level: %d"), NewLevel);
	
	// Get upgrade subsystem
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogXP, Error, TEXT("[MyCharacter] Cannot show level up modal - World is null"));
		return;
	}
	
	UUpgradeSubsystem* UpgradeSS = World->GetSubsystem<UUpgradeSubsystem>();
	if (!UpgradeSS)
	{
		UE_LOG(LogXP, Error, TEXT("[MyCharacter] Cannot show level up modal - UpgradeSubsystem not found"));
		return;
	}
	
	// Generate random upgrades
	TArray<FUpgradeData> RandomUpgrades = UpgradeSS->GenerateRandomUpgrades(3);
	if (RandomUpgrades.Num() == 0)
	{
		UE_LOG(LogXP, Warning, TEXT("[MyCharacter] No upgrades available!"));
		return;
	}
	
	UE_LOG(LogXP, Display, TEXT("[MyCharacter] Generated %d upgrade options:"), RandomUpgrades.Num());
	for (const FUpgradeData& Upgrade : RandomUpgrades)
	{
		UE_LOG(LogXP, Display, TEXT("  - %s (Level %d/%d)"), 
			*Upgrade.DisplayName.ToString(), 
			Upgrade.CurrentLevel, 
			Upgrade.MaxLevel);
	}
	
	// Show level up modal
	ShowLevelUpModal(RandomUpgrades);
}

void AMyCharacter::ShowLevelUpModal(const TArray<FUpgradeData>& Upgrades)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		UE_LOG(LogXP, Error, TEXT("[MyCharacter] Cannot show modal - PlayerController is null"));
		return;
	}
	
	// Close existing modal if any
	if (ActiveLevelUpModal.IsValid())
	{
		CloseLevelUpModal();
	}
	
	// Pause game
	PC->SetPause(true);
	UE_LOG(LogXP, Display, TEXT("[MyCharacter] Game paused for level up"));
	
	// Set input mode to UI only
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = true;
	
	// Create Slate modal
	ActiveLevelUpModal = SNew(SLevelUpModal)
		.OnUpgradeChosen_Lambda([this](EUpgradeType ChosenType)
		{
			OnUpgradeChosen(ChosenType);
		});
	
	// Setup upgrades
	ActiveLevelUpModal->SetupUpgrades(Upgrades);
	
	// Add to viewport
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->AddViewportWidgetContent(
			ActiveLevelUpModal.ToSharedRef(),
			100 // High Z-order to be on top
		);
		
		// Force focus to the modal
		FSlateApplication::Get().SetKeyboardFocus(ActiveLevelUpModal);
		
		UE_LOG(LogXP, Display, TEXT("[MyCharacter] ✅ Level Up modal displayed!"));
	}
	else
	{
		UE_LOG(LogXP, Error, TEXT("[MyCharacter] Failed to add modal to viewport - GEngine or GameViewport is null"));
		PC->SetPause(false); // Unpause if we failed
	}
}

void AMyCharacter::OnUpgradeChosen(EUpgradeType ChosenType)
{
	UE_LOG(LogXP, Warning, TEXT("[MyCharacter] 🎯 Upgrade chosen: %d"), (int32)ChosenType);
	
	// Apply upgrade
	if (UWorld* World = GetWorld())
	{
		if (UUpgradeSubsystem* UpgradeSS = World->GetSubsystem<UUpgradeSubsystem>())
		{
			UpgradeSS->ApplyUpgrade(ChosenType, this);
			
			// Get upgrade info for feedback
			FUpgradeData UpgradeInfo = UpgradeSS->GetUpgradeDisplayData(ChosenType);
			UE_LOG(LogXP, Display, TEXT("[MyCharacter] ✨ Applied upgrade: %s (Level %d)"), 
				*UpgradeInfo.DisplayName.ToString(),
				UpgradeSS->GetUpgradeLevel(ChosenType));
		}
	}
	
	// Close modal and resume game
	CloseLevelUpModal();
}

void AMyCharacter::CloseLevelUpModal()
{
	if (!ActiveLevelUpModal.IsValid())
	{
		return;
	}
	
	// Remove from viewport
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(ActiveLevelUpModal.ToSharedRef());
		UE_LOG(LogXP, Display, TEXT("[MyCharacter] Level Up modal closed"));
	}
	
	// Reset modal reference
	ActiveLevelUpModal.Reset();
	
	// Unpause game and restore input mode
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		PC->SetPause(false);
		
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
		
		UE_LOG(LogXP, Display, TEXT("[MyCharacter] Game resumed"));
	}
}
