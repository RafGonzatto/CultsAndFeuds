#include "World/Common/GameModes/CityGameMode.h"
#include "World/Common/Player/MyCharacter.h"
#include "World/Common/Player/MyPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"

ACityGameMode::ACityGameMode()
{
	DefaultPawnClass = AMyCharacter::StaticClass();
	PlayerControllerClass = AMyPlayerController::StaticClass();

	// Carregar assets no constructor (onde FObjectFinder é permitido)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMeshAsset.Succeeded())
	{
		CubeMesh = CubeMeshAsset.Object;
		UE_LOG(LogTemp, Warning, TEXT("[CityGameMode] CubeMesh carregado: %s"), *CubeMesh->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[CityGameMode] FALHA ao carregar CubeMesh!"));
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> BasicMaterialAsset(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (BasicMaterialAsset.Succeeded())
	{
		BasicMaterial = BasicMaterialAsset.Object;
		UE_LOG(LogTemp, Warning, TEXT("[CityGameMode] BasicMaterial carregado: %s"), *BasicMaterial->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[CityGameMode] FALHA ao carregar BasicMaterial!"));
	}

	UE_LOG(LogTemp, Warning, TEXT("[CityGameMode] Construtor - Assets carregados: Mesh=%s, Material=%s"), 
		CubeMesh ? TEXT("OK") : TEXT("FAIL"),
		BasicMaterial ? TEXT("OK") : TEXT("FAIL"));
}

void ACityGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning, TEXT("[CityGameMode] BeginPlay - Criando ambiente da cidade"));
	
	// Criar PlayerStart PRIMEIRO, antes de tudo
	CreatePlayerStartIfNeeded();
	
	// Dar tempo para o PlayerStart ser registrado no sistema
	GetWorld()->GetTimerManager().SetTimer(
		SpawnDelayTimer,
		FTimerDelegate::CreateUObject(this, &ACityGameMode::CreateEnvironmentDelayed),
		0.1f, // 100ms de delay
		false
	);
}

void ACityGameMode::CreateEnvironmentDelayed()
{
	// Criar o resto do ambiente após PlayerStart estar pronto
	CreateCityGround();
	CreateBasicLighting();
	
	UE_LOG(LogTemp, Warning, TEXT("[CityGameMode] Ambiente da cidade criado com delay"));
}

void ACityGameMode::CreatePlayerStartIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Verificar se já existe um PlayerStart
	for (TActorIterator<APlayerStart> ActorItr(World); ActorItr; ++ActorItr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CityGameMode] PlayerStart já existe"));
		return;
	}

	// Criar PlayerStart no centro da cidade com rotação e configurações adequadas
	FVector SpawnLocation(0.0f, 0.0f, 150.0f); // Mais alto para garantir spawn seguro
	FRotator SpawnRotation(0.0f, 0.0f, 0.0f);
	
	APlayerStart* PlayerStart = World->SpawnActor<APlayerStart>(SpawnLocation, SpawnRotation);
	if (PlayerStart)
	{
		PlayerStart->SetActorLabel(TEXT("CityPlayerStart"));
		
		UE_LOG(LogTemp, Warning, TEXT("[CityGameMode] PlayerStart criado e configurado em (0,0,150)"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[CityGameMode] FALHA ao criar PlayerStart!"));
	}
}

void ACityGameMode::CreateCityGround()
{
	UWorld* World = GetWorld();
	if (!World) 
	{
		UE_LOG(LogTemp, Error, TEXT("[CityGameMode] World é NULL"));
		return;
	}

	if (!CubeMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("[CityGameMode] CubeMesh não foi carregado!"));
		return;
	}

	// Criar plano grande para andar (30x30 metros)
	FVector GroundLocation(0.0f, 0.0f, 0.0f);
	FRotator GroundRotation(0.0f, 0.0f, 0.0f);
	
	AStaticMeshActor* GroundActor = World->SpawnActor<AStaticMeshActor>(GroundLocation, GroundRotation);
	if (GroundActor)
	{
		UStaticMeshComponent* GroundMesh = GroundActor->GetStaticMeshComponent();
		
		// CORRIGIR MOBILITY ANTES de definir o mesh
		GroundMesh->SetMobility(EComponentMobility::Movable);
		
		// Agora sim definir o mesh
		GroundMesh->SetStaticMesh(CubeMesh);
		
		// Fazer scale para criar um plano: 30x30x0.2 metros
		GroundActor->SetActorScale3D(FVector(30.0f, 30.0f, 0.2f));
		
		// Aplicar cor verde para o chão ser visível
		if (BasicMaterial)
		{
			// Criar uma instância dinâmica do material para mudar a cor
			UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BasicMaterial, GroundActor);
			if (DynamicMaterial)
			{
				// Definir cor verde para o chão
				DynamicMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.2f, 0.8f, 0.2f, 1.0f));
				GroundMesh->SetMaterial(0, DynamicMaterial);
			}
		}
		
		GroundActor->SetActorLabel(TEXT("CityGround"));
		UE_LOG(LogTemp, Warning, TEXT("[CityGameMode] Chão VERDE da cidade criado: 30x30m - Posição: %s"), *GroundLocation.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[CityGameMode] Falha ao criar GroundActor"));
	}

	// Criar alguns cubos de referência coloridos
	CreateReferenceCubes();
}

void ACityGameMode::CreateReferenceCubes()
{
	UWorld* World = GetWorld();
	if (!World || !CubeMesh || !BasicMaterial) return;

	// Criar 4 cubos coloridos para referência visual
	TArray<FVector> CubePositions = {
		FVector(500.0f, 500.0f, 100.0f),   // Nordeste - Azul
		FVector(-500.0f, 500.0f, 100.0f),  // Noroeste - Vermelho  
		FVector(500.0f, -500.0f, 100.0f),  // Sudeste - Amarelo
		FVector(-500.0f, -500.0f, 100.0f)  // Sudoeste - Roxo
	};
	
	TArray<FLinearColor> CubeColors = {
		FLinearColor(0.2f, 0.2f, 1.0f, 1.0f), // Azul
		FLinearColor(1.0f, 0.2f, 0.2f, 1.0f), // Vermelho
		FLinearColor(1.0f, 1.0f, 0.2f, 1.0f), // Amarelo
		FLinearColor(0.8f, 0.2f, 0.8f, 1.0f)  // Roxo
	};

	for (int32 i = 0; i < CubePositions.Num(); i++)
	{
		AStaticMeshActor* CubeActor = World->SpawnActor<AStaticMeshActor>(CubePositions[i], FRotator::ZeroRotator);
		if (CubeActor)
		{
			UStaticMeshComponent* CubeMeshComp = CubeActor->GetStaticMeshComponent();
			
			// CORRIGIR MOBILITY ANTES de definir o mesh
			CubeMeshComp->SetMobility(EComponentMobility::Movable);
			
			// Agora sim definir o mesh
			CubeMeshComp->SetStaticMesh(CubeMesh);
			CubeActor->SetActorScale3D(FVector(2.0f, 2.0f, 2.0f)); // Cubos de 200x200x200cm
			
			// Aplicar cor específica
			UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BasicMaterial, CubeActor);
			if (DynamicMaterial)
			{
				DynamicMaterial->SetVectorParameterValue(TEXT("Color"), CubeColors[i]);
				CubeMeshComp->SetMaterial(0, DynamicMaterial);
			}
			
			CubeActor->SetActorLabel(FString::Printf(TEXT("RefCube_%d"), i));
			UE_LOG(LogTemp, Warning, TEXT("[CityGameMode] Cubo %d criado na posição: %s"), i, *CubePositions[i].ToString());
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[CityGameMode] 4 cubos de referência coloridos criados com Mobility corrigida"));
}

void ACityGameMode::CreateBasicLighting()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Criar Directional Light (sol)
	FVector LightLocation(0.0f, 0.0f, 1000.0f);
	FRotator LightRotation(-45.0f, 45.0f, 0.0f); // 45 graus para baixo, 45 graus de lado
	
	ADirectionalLight* DirectionalLight = World->SpawnActor<ADirectionalLight>(LightLocation, LightRotation);
	if (DirectionalLight)
	{
		UDirectionalLightComponent* LightComponent = DirectionalLight->GetComponent();
		LightComponent->SetIntensity(5.0f); // Bem brilhante
		LightComponent->SetLightColor(FLinearColor(1.0f, 0.9f, 0.8f, 1.0f)); // Cor de sol
		
		DirectionalLight->SetActorLabel(TEXT("CityDirectionalLight"));
		UE_LOG(LogTemp, Warning, TEXT("[CityGameMode] Directional Light criado"));
	}

	// Criar Sky Light para iluminação ambiente
	FVector SkyLightLocation(0.0f, 0.0f, 800.0f);
	ASkyLight* SkyLight = World->SpawnActor<ASkyLight>(SkyLightLocation, FRotator::ZeroRotator);
	if (SkyLight)
	{
		USkyLightComponent* SkyLightComponent = SkyLight->GetLightComponent();
		SkyLightComponent->SetIntensity(2.0f);
		SkyLightComponent->SetLightColor(FLinearColor(0.5f, 0.7f, 1.0f, 1.0f)); // Azul claro
		
		SkyLight->SetActorLabel(TEXT("CitySkyLight"));
		UE_LOG(LogTemp, Warning, TEXT("[CityGameMode] Sky Light criado"));
	}
}

AActor* ACityGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	UWorld* World = GetWorld();
	if (World)
	{
		// Procurar especificamente pelo nosso PlayerStart
		for (TActorIterator<APlayerStart> ActorItr(World); ActorItr; ++ActorItr)
		{
			APlayerStart* PlayerStart = *ActorItr;
			if (PlayerStart)
			{
				// Usar qualquer PlayerStart que encontrarmos (inclusive o nosso)
				UE_LOG(LogTemp, Warning, TEXT("[CityGameMode] Encontrou PlayerStart: %s na posicao: %s"), 
					*PlayerStart->GetName(), 
					*PlayerStart->GetActorLocation().ToString());
				return PlayerStart;
			}
		}
	}
	
	// Se não encontrou nenhum, forçar criação de um temporário
	UE_LOG(LogTemp, Error, TEXT("[CityGameMode] NENHUM PlayerStart encontrado! Criando um temporario"));
	CreatePlayerStartIfNeeded();
	
	// Tentar novamente após criar
	if (World)
	{
		for (TActorIterator<APlayerStart> ActorItr(World); ActorItr; ++ActorItr)
		{
			APlayerStart* PlayerStart = *ActorItr;
			if (PlayerStart)
			{
				UE_LOG(LogTemp, Warning, TEXT("[CityGameMode] Usando PlayerStart recem-criado"));
				return PlayerStart;
			}
		}
	}
	
	UE_LOG(LogTemp, Error, TEXT("[CityGameMode] FALHOU completamente em encontrar PlayerStart!"));
	return Super::ChoosePlayerStart_Implementation(Player);
}

void ACityGameMode::CreateCityBounds()
{
	// Removido por enquanto para simplificar debug
}

void ACityGameMode::CreateBoundaryWall(FVector Location, FVector Scale)
{
	// Removido por enquanto para simplificar debug  
}
