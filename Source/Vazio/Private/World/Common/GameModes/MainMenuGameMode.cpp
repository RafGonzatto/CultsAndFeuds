#include "World/Common/GameModes/MainMenuGameMode.h"
#include "Logging/VazioLogFacade.h"
#include "World/Common/Player/MainMenuPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Camera/CameraActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"

AMainMenuGameMode::AMainMenuGameMode()
{
    // Main Menu não precisa de Pawn, Player Controller customizado, etc
    DefaultPawnClass = nullptr;
    PlayerControllerClass = AMainMenuPlayerController::StaticClass();
}

void AMainMenuGameMode::BeginPlay()
{
    Super::BeginPlay();

        // 1. Criar câmera básica para renderizar algo
    CreateMenuCamera();
    
    // UI é criada no PlayerController local

    LOG_UI(Info, TEXT("[MainMenu] Main Menu initialized successfully"));
}


void AMainMenuGameMode::CreateMenuCamera()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // 1. Criar Sky Atmosphere moderno para iluminação ambiente
    if (UClass* SkyAtmosClass = StaticLoadClass(AActor::StaticClass(), nullptr, TEXT("/Script/Engine.SkyAtmosphere")))
    {
        if (AActor* SkyAtmos = World->SpawnActor<AActor>(SkyAtmosClass))
        {
            LOG_UI(Info, TEXT("[MainMenu] SkyAtmosphere created"));
        }
    }
    
    // 2. Criar Exponential Height Fog para profundidade
    AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>();
    if (Fog)
    {
        if (auto* FogComp = Fog->GetComponent())
        {
            FogComp->SetFogDensity(0.002f);
        }
    LOG_UI(Warn, TEXT("[MainMenu] Exponential Height Fog created"));
    }
    
    // 3. Criar luz direcional para iluminar a cena
    ADirectionalLight* Light = World->SpawnActor<ADirectionalLight>(FVector::ZeroVector, FRotator(-45.f, 0.f, 0.f));
    if (Light)
    {
        Light->SetBrightness(10.0f); // Aumentando o brilho
    LOG_UI(Warn, TEXT("[MainMenu] Directional light created with brightness 10"));
    }

    // 4. Spawnar cubo com material colorido PRIMEIRO (antes da câmera)
    FVector CubeLocation(0.f, 200.f, 50.f);  // Um pouco acima do chão e à frente
    FRotator CubeRotation(0.f, 45.f, 0.f);
    AStaticMeshActor* TestCube = World->SpawnActor<AStaticMeshActor>(CubeLocation, CubeRotation);
    if (TestCube && TestCube->GetStaticMeshComponent())
    {
        UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
        if (CubeMesh)
        {
            TestCube->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
            TestCube->SetActorScale3D(FVector(2.0f, 2.0f, 2.0f));
            
            // Criar material básico colorido
            UMaterial* BaseMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
            if (BaseMaterial)
            {
                UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMaterial, TestCube);
                if (DynMat)
                {
                    DynMat->SetVectorParameterValue(FName("Color"), FLinearColor(1.0f, 0.3f, 0.3f, 1.0f)); // Vermelho
                    TestCube->GetStaticMeshComponent()->SetMaterial(0, DynMat);
                    LOG_UI(Warn, TEXT("[MainMenu] Test cube at (%.1f, %.1f, %.1f) with RED material created!"),
                        CubeLocation.X, CubeLocation.Y, CubeLocation.Z);
                }
            }
        }
    }
    
    // 5. Criar câmera olhando para o cubo
    FVector CameraLocation(0.f, -300.f, 150.f);  // Atrás e um pouco acima
    // Calcular rotação apontando para o cubo
    FRotator CameraRotation = (CubeLocation - CameraLocation).Rotation();

    ACameraActor* MenuCamera = World->SpawnActor<ACameraActor>(CameraLocation, CameraRotation);
    if (MenuCamera)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
        if (PC)
        {
            PC->SetViewTargetWithBlend(Cast<AActor>(MenuCamera), 0.f);
            LOG_UI(Warn, TEXT("[MainMenu] Camera at (%.1f, %.1f, %.1f) looking at cube at (%.1f, %.1f, %.1f)"),
                CameraLocation.X, CameraLocation.Y, CameraLocation.Z, CubeLocation.X, CubeLocation.Y, CubeLocation.Z);
        }
    }
}


// UI creation and input setup handled by AMainMenuPlayerController