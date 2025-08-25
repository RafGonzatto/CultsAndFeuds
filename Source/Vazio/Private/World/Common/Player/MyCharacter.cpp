#include "World/Common/Player/MyCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

AMyCharacter::AMyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Capsule
	GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);
	GetCapsuleComponent()->SetCapsuleRadius(34.0f);

	// Mesh/Skeletal fallback (unchanged)
	USkeletalMeshComponent* MeshComp = GetMesh();
	MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	// Tentar carregar um SkeletalMesh (pode falhar)
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Engine/VREditor/Devices/Vive/VivePreControllerMesh"));
	if (MeshAsset.Succeeded())
	{
		MeshComp->SetSkeletalMesh(MeshAsset.Object);
		UE_LOG(LogTemp, Warning, TEXT("[MyCharacter] SkeletalMesh carregado com sucesso"));
	}
	else
	{
		// FALLBACK: Criar um cubo visual para representar o player
		VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
		VisualMesh->SetupAttachment(RootComponent);
		
		// Carregar cubo básico para visualização
		static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
		if (CubeMeshAsset.Succeeded())
		{
			VisualMesh->SetStaticMesh(CubeMeshAsset.Object);
			VisualMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
			VisualMesh->SetRelativeScale3D(FVector(0.7f, 0.7f, 1.5f)); // Formato humanóide
			
			// Carregar material básico
			static ConstructorHelpers::FObjectFinder<UMaterial> BasicMaterialAsset(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
			if (BasicMaterialAsset.Succeeded())
			{
				VisualMesh->SetMaterial(0, BasicMaterialAsset.Object);
			}
		}
		
		UE_LOG(LogTemp, Warning, TEXT("[MyCharacter] Usando cubo visual como representação do player"));
	}

	// SpringArm para câmera TOP-DOWN ANGULADA (estilo Diablo)
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 600.f; // Distância moderada
	SpringArm->bUsePawnControlRotation = false; // Câmera fixa
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f)); // Altura dos ombros
	
	// ÂNGULO TOP-DOWN: 45° para baixo + rotação lateral
	SpringArm->SetRelativeRotation(FRotator(-45.0f, -45.0f, 0.0f));
	
	// Configurações adicionais do SpringArm
	SpringArm->bDoCollisionTest = true; // Evitar atravessar paredes
	SpringArm->bUsePawnControlRotation = false; // Câmera não rotaciona com mouse
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;
	SpringArm->SetUsingAbsoluteRotation(true); // Manter orientação da câmera fixa, independentemente da rotação do pawn

	// Câmera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	// Configurar movimento - VELOCIDADE MODERADA
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 720.f, 0.f);
	Movement->MaxWalkSpeed = 400.0f; // Velocidade normal
	Movement->BrakingDecelerationWalking = 2000.0f;

	// Não usar rotação do controller para Yaw (deixar para o movement)
	bUseControllerRotationYaw = false;

	UE_LOG(LogTemp, Warning, TEXT("[MyCharacter] Camera top-down FIXA: absolute rotation habilitada"));
}

void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Se temos um VisualMesh, vamos colorir para ser mais visível
	if (VisualMesh)
	{
		UMaterialInterface* CurrentMaterial = VisualMesh->GetMaterial(0);
		if (CurrentMaterial)
		{
			UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(CurrentMaterial, this);
			if (DynamicMaterial)
			{
				// Colorir o player de azul brilhante
				DynamicMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.2f, 0.5f, 1.0f, 1.0f));
				VisualMesh->SetMaterial(0, DynamicMaterial);
			}
		}
	}

	// 1) Snap para o chão (garantir modo WALKING)
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	const float HalfHeight = Capsule ? Capsule->GetScaledCapsuleHalfHeight() : 88.f;
	FVector Start = GetActorLocation() + FVector(0, 0, 50);
	FVector End = Start - FVector(0, 0, 1000);
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SnapToGround), false, this);
	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
	if (bHit)
	{
		FVector NewLoc = Hit.ImpactPoint + FVector(0, 0, HalfHeight + 2.f);
		SetActorLocation(NewLoc);
		UE_LOG(LogTemp, Warning, TEXT("[MyCharacter] SnapToGround -> %s (hit %s)"), *NewLoc.ToString(), *Hit.GetActor()->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[MyCharacter] SnapToGround: nenhum hit, mantendo posição %s"), *GetActorLocation().ToString());
	}

	// 2) Garantir modo WALKING
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->SetMovementMode(MOVE_Walking);
		UE_LOG(LogTemp, Warning, TEXT("[MyCharacter] MovementMode = %d Vel=%s"), (int32)Move->MovementMode, *Move->Velocity.ToString());
	}
}
