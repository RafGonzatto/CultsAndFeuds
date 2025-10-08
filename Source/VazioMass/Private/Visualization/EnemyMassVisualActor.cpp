#include "Visualization/EnemyMassVisualActor.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    constexpr float CubeHalfExtent = 50.0f; // /Engine/BasicShapes/Cube has 100 uu height centered at origin.
    const FVector BaseVisualScale(0.75f, 0.75f, 1.40f);
}

namespace
{
    static FLinearColor GetColorForArchetype(const FName ArchetypeName)
    {
        const FString Name = ArchetypeName.ToString();
        if (Name.Contains(TEXT("Heavy")))
        {
            return FLinearColor(0.122f, 0.435f, 0.839f);
        }
        if (Name.Contains(TEXT("Ranged")))
        {
            return FLinearColor(0.992f, 0.812f, 0.082f);
        }
        if (Name.Contains(TEXT("Dash")))
        {
            return FLinearColor(0.090f, 0.890f, 0.447f);
        }
        if (Name.Contains(TEXT("Aura")))
        {
            return FLinearColor(0.620f, 0.220f, 0.878f);
        }
        if (Name.Contains(TEXT("Gold")))
        {
            return FLinearColor(0.992f, 0.858f, 0.082f);
        }
        if (Name.Contains(TEXT("Splitter")) || Name.Contains(TEXT("Slime")))
        {
            return FLinearColor(0.984f, 0.502f, 0.090f);
        }
        if (Name.Contains(TEXT("Boss")))
        {
            return FLinearColor(0.839f, 0.149f, 0.188f);
        }
        return FLinearColor(0.992f, 0.243f, 0.318f);
    }
}

AEnemyMassVisualActor::AEnemyMassVisualActor()
{
#if WITH_EDITOR
    SetActorLabel(TEXT("EnemyMassVisual"));
#endif

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    VisualComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
    VisualComponent->SetupAttachment(RootComponent);
    VisualComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    VisualComponent->SetGenerateOverlapEvents(false);
    VisualComponent->SetMobility(EComponentMobility::Movable);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Engine/BasicShapes/Cube"));
    if (MeshFinder.Succeeded())
    {
        VisualComponent->SetStaticMesh(MeshFinder.Object);
        VisualComponent->SetRelativeScale3D(BaseVisualScale);
        VisualComponent->SetRelativeLocation(FVector(0.0f, 0.0f, CubeHalfExtent * BaseVisualScale.Z));
    }

    SetActorEnableCollision(false);
    SetCanBeDamaged(false);
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;
}

void AEnemyMassVisualActor::BeginPlay()
{
    Super::BeginPlay();
    EnsureMaterial();
}

void AEnemyMassVisualActor::ApplyArchetypeStyle(const FName ArchetypeName, const FVector& VisualScale)
{
    CachedArchetype = ArchetypeName;
    const FVector FinalScale = FVector(VisualScale.X * BaseVisualScale.X, VisualScale.Y * BaseVisualScale.Y, VisualScale.Z * BaseVisualScale.Z);
    VisualComponent->SetRelativeScale3D(FinalScale);
    VisualComponent->SetRelativeLocation(FVector(0.0f, 0.0f, CubeHalfExtent * FinalScale.Z));
    VisualComponent->SetRelativeRotation(FRotator::ZeroRotator);
    EnsureMaterial();
    ApplyColor(GetColorForArchetype(ArchetypeName));
    SetVisualActive(true);
}

void AEnemyMassVisualActor::ApplyTransform(const FTransform& InTransform)
{
    FVector Location = InTransform.GetLocation();
    Location.Z = FMath::Max(Location.Z, 0.0f);

    const float Yaw = InTransform.GetRotation().Rotator().Yaw;
    SetActorLocation(Location);
    SetActorRotation(FRotator(0.0f, Yaw, 0.0f));
}

void AEnemyMassVisualActor::SetVisualActive(const bool bActive)
{
    SetActorHiddenInGame(!bActive);
    SetActorTickEnabled(false);
}

void AEnemyMassVisualActor::EnsureMaterial()
{
    if (!VisualComponent)
    {
        return;
    }

    if (!DynamicMaterial)
    {
        UMaterialInterface* BaseMaterial = VisualComponent->GetMaterial(0);
        if (!BaseMaterial)
        {
            static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/EngineMaterials/DefaultMaterial"));
            if (MaterialFinder.Succeeded())
            {
                BaseMaterial = MaterialFinder.Object;
            }
        }

        if (BaseMaterial)
        {
            DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            if (DynamicMaterial)
            {
                VisualComponent->SetMaterial(0, DynamicMaterial);
            }
        }
    }
}

void AEnemyMassVisualActor::ApplyColor(const FLinearColor& Color)
{
    if (DynamicMaterial)
    {
        DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
        DynamicMaterial->SetScalarParameterValue(TEXT("EmissiveStrength"), 4.0f);
    }
}
