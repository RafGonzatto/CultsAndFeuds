// CityGameMode.cpp
#include "World/Common/GameModes/CityGameMode.h"
#include "World/Common/Player/MyCharacter.h"
#include "World/Common/Player/MyPlayerController.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"

ACityGameMode::ACityGameMode()
{
	// Default classes (fallback em C++)
	DefaultPawnClass     = AMyCharacter::StaticClass();
	PlayerControllerClass = AMyPlayerController::StaticClass();

	// Preferir BP se existir
	static ConstructorHelpers::FClassFinder<APawn> BPCharClass(TEXT("/Game/Characters/PlayerChar/BP_MyCharacter"));
	if (BPCharClass.Succeeded())
	{
		DefaultPawnClass = BPCharClass.Class;
		UE_LOG(LogTemp, Log, TEXT("[CityGM] DefaultPawnClass = BP_MyCharacter"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[CityGM] BP_MyCharacter nao encontrado. Usando AMyCharacter."));
	}

	// Assets básicos do Engine
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
	CubeMesh = CubeMeshAsset.Succeeded() ? CubeMeshAsset.Object : nullptr;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMatAsset(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	BasicMaterial = BasicMatAsset.Succeeded() ? BasicMatAsset.Object : nullptr;

	UE_LOG(LogTemp, Log, TEXT("[CityGM] Assets: CubeMesh=%s, BasicMaterial=%s"),
		CubeMesh ? TEXT("OK") : TEXT("FAIL"),
		BasicMaterial ? TEXT("OK") : TEXT("FAIL"));
}

void ACityGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    // Garante PlayerStart antes do processo de spawn do player
    CreatePlayerStartIfNeeded();
    Super::InitGame(MapName, Options, ErrorMessage);
}

void ACityGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("[CityGM] BeginPlay. DefaultPawn=%s"),
		*GetDefaultPawnClassForController(nullptr)->GetName());

	CreatePlayerStartIfNeeded();

	// Cria o chão imediatamente para evitar que o player caia antes do primeiro tick
	CreateCityGround();

	// Iluminação e outras coisas leves no próximo tick
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ACityGameMode::CreateEnvironmentNextTick);
}

void ACityGameMode::CreateEnvironmentNextTick()
{
	CreateBasicLighting();
	// CreateCityBounds(); // mantido desativado por enquanto
	UE_LOG(LogTemp, Log, TEXT("[CityGM] Ambiente criado (next tick)."));
}

void ACityGameMode::CreatePlayerStartIfNeeded()
{
	UWorld* World = GetWorld(); if (!World) return;

	// Já existe algum com nossa tag?
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		if (It->ActorHasTag(GetCitySpawnTag()))
		{
			UE_LOG(LogTemp, Log, TEXT("[CityGM] PlayerStart com tag encontrado: %s"), *It->GetName());
			return;
		}
	}

	// Cria no centro
	const FVector  Loc(0.f, 0.f, 150.f);
	const FRotator Rot = FRotator::ZeroRotator;
	APlayerStart* PS = World->SpawnActor<APlayerStart>(Loc, Rot);
	if (PS)
	{
#if WITH_EDITOR
		PS->SetActorLabel(TEXT("CityPlayerStart"));
#endif
		PS->Tags.AddUnique(GetCitySpawnTag());
		UE_LOG(LogTemp, Log, TEXT("[CityGM] PlayerStart criado em (0,0,150) com tag CitySpawn."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[CityGM] Falha ao criar PlayerStart."));
	}
}

AActor* ACityGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	UWorld* World = GetWorld(); if (!World) return Super::ChoosePlayerStart_Implementation(Player);

	// Preferir por tag determinística
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		if (It->ActorHasTag(GetCitySpawnTag()))
		{
			UE_LOG(LogTemp, Log, TEXT("[CityGM] Usando PlayerStart com tag CitySpawn: %s"), *It->GetName());
			return *It;
		}
	}

	// Fallback: qualquer PlayerStart
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CityGM] Usando PlayerStart generico: %s"), *It->GetName());
		return *It;
	}

	UE_LOG(LogTemp, Error, TEXT("[CityGM] Nenhum PlayerStart encontrado. Retornando Super."));
	return Super::ChoosePlayerStart_Implementation(Player);
}

void ACityGameMode::CreateCityGround()
{
	if (!CubeMesh) { UE_LOG(LogTemp, Error, TEXT("[CityGM] CubeMesh nao carregado.")); return; }

	// 30x30m, fino em Z
	AStaticMeshActor* Ground = SpawnMeshActor(FVector::ZeroVector, FRotator::ZeroRotator, FVector(30.f, 30.f, 0.2f));
	if (!Ground) { UE_LOG(LogTemp, Error, TEXT("[CityGM] Falha ao criar Ground.")); return; }

	UStaticMeshComponent* Mesh = Ground->GetStaticMeshComponent();
	// Primeiro mobilidade, depois troca de mesh, para evitar warnings
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetStaticMesh(CubeMesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);

	ApplyColorMID(Mesh, FLinearColor(0.2f, 0.8f, 0.2f)); // verde
#if WITH_EDITOR
	Ground->SetActorLabel(TEXT("CityGround"));
#endif
	UE_LOG(LogTemp, Log, TEXT("[CityGM] Ground criado 30x30m."));
	CreateReferenceCubes();
}

void ACityGameMode::CreateReferenceCubes()
{
	if (!CubeMesh || !BasicMaterial) return;

	const TArray<FVector> Positions{
		{  500.f,  500.f, 100.f }, // NE
		{ -500.f,  500.f, 100.f }, // NO
		{  500.f, -500.f, 100.f }, // SE
		{ -500.f, -500.f, 100.f }  // SO
	};
	const TArray<FLinearColor> Colors{
		{0.2f,0.2f,1.0f,1.0f}, // Azul
		{1.0f,0.2f,0.2f,1.0f}, // Vermelho
		{1.0f,1.0f,0.2f,1.0f}, // Amarelo
		{0.8f,0.2f,0.8f,1.0f}  // Roxo
	};

	for (int32 i = 0; i < Positions.Num(); ++i)
	{
		AStaticMeshActor* Cube = SpawnMeshActor(Positions[i], FRotator::ZeroRotator, FVector(2.f));
		if (!Cube) continue;

		UStaticMeshComponent* C = Cube->GetStaticMeshComponent();
		// Primeiro mobilidade, depois mesh
		C->SetMobility(EComponentMobility::Movable);
		C->SetStaticMesh(CubeMesh);
		ApplyColorMID(C, Colors[i]);
#if WITH_EDITOR
		Cube->SetActorLabel(*FString::Printf(TEXT("RefCube_%d"), i));
#endif
		UE_LOG(LogTemp, Log, TEXT("[CityGM] RefCube_%d criado em %s"), i, *Positions[i].ToString());
	}

	UE_LOG(LogTemp, Log, TEXT("[CityGM] 4 cubos de referencia criados."));
}

void ACityGameMode::CreateBasicLighting()
{
	// Evitar spawn de luzes no servidor dedicado
	if (IsRunningDedicatedServer()) return;

	UWorld* World = GetWorld(); if (!World) return;

	// Directional Light (sol)
	if (ADirectionalLight* DL = World->SpawnActor<ADirectionalLight>(FVector(0,0,1000), FRotator(-45,45,0)))
	{
		if (UDirectionalLightComponent* C = Cast<UDirectionalLightComponent>(DL->GetLightComponent()))
		{
			C->SetMobility(EComponentMobility::Movable);
			C->SetIntensity(5.0f);
			C->SetLightColor(FLinearColor(1.0f, 0.9f, 0.8f));
		}
#if WITH_EDITOR
		DL->SetActorLabel(TEXT("CityDirectionalLight"));
#endif
	}

	// SkyLight (ambiente)
	if (ASkyLight* SL = World->SpawnActor<ASkyLight>(FVector(0,0,800), FRotator::ZeroRotator))
	{
		if (USkyLightComponent* C = Cast<USkyLightComponent>(SL->GetLightComponent()))
		{
			C->SetMobility(EComponentMobility::Movable);
			C->SetIntensity(2.0f);
			C->SetLightColor(FLinearColor(0.5f, 0.7f, 1.0f));
			C->RecaptureSky();
		}
#if WITH_EDITOR
		SL->SetActorLabel(TEXT("CitySkyLight"));
#endif
	}

	UE_LOG(LogTemp, Log, TEXT("[CityGM] Iluminacao basica criada."));
}

void ACityGameMode::CreateCityBounds() { /* mantido desativado */ }
void ACityGameMode::CreateBoundaryWall(FVector, FVector) { /* mantido desativado */ }

AStaticMeshActor* ACityGameMode::SpawnMeshActor(const FVector& Loc, const FRotator& Rot, const FVector& Scale) const
{
	UWorld* World = GetWorld(); if (!World) return nullptr;
	AStaticMeshActor* A = World->SpawnActor<AStaticMeshActor>(Loc, Rot);
	if (A)
	{
		A->SetActorScale3D(Scale);
		if (UStaticMeshComponent* C = A->GetStaticMeshComponent())
		{
			C->SetMobility(EComponentMobility::Movable);
		}
	}
	return A;
}

void ACityGameMode::ApplyColorMID(UPrimitiveComponent* Comp, const FLinearColor& Color) const
{
	if (!Comp || !BasicMaterial) return;
	if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BasicMaterial, Comp))
	{
		// BasicShapeMaterial expõe vetor "Color"
		MID->SetVectorParameterValue(TEXT("Color"), Color);
		Comp->SetMaterial(0, MID);
	}
}
