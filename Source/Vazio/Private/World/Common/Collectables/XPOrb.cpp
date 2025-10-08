#include "World/Common/Collectables/XPOrb.h"
#include "Logging/VazioLogFacade.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "World/Common/Player/XPComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

AXPOrb::AXPOrb() {
    PrimaryActorTick.bCanEverTick = true;

    Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
    RootComponent = Sphere;
    Sphere->InitSphereRadius(32.f);
    Sphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    Sphere->SetGenerateOverlapEvents(true);
    Sphere->OnComponentBeginOverlap.AddDynamic(this, &AXPOrb::OnOverlap);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(RootComponent);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // Configurar atributos visuais para destacar os orbes
    Mesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
    
    // Tentativa de carregar um mesh b�sico para o XPOrb
    static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (DefaultMesh.Succeeded()) {
        Mesh->SetStaticMesh(DefaultMesh.Object);
    }
}

void AXPOrb::BeginPlay() {
    Super::BeginPlay();
    TargetPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    LOG_UPGRADES(Info, TEXT("XPOrb spawned at %s with %d XP"), *GetActorLocation().ToString(), XPAmount);
    
    // Criar material din�mico com cor brilhante para destacar o orbe
    if (Mesh && Mesh->GetStaticMesh()) {
        if (Mesh->GetMaterial(0)) {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Mesh->GetMaterial(0), this);
            if (DynamicMaterial) {
                DynamicMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.2f, 0.8f, 1.0f));
                DynamicMaterial->SetScalarParameterValue(TEXT("Emissive"), 5.0f);
                Mesh->SetMaterial(0, DynamicMaterial);
            }
        }
    }
}

void AXPOrb::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
    if (!TargetPlayer) {
        TargetPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (!TargetPlayer) return;
    }
    
    const float Dist = FVector::Dist(TargetPlayer->GetActorLocation(), GetActorLocation());
    if (Dist <= AttractionRadius) {
        const FVector Dir = (TargetPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        // Aumenta a velocidade quanto mais pr�ximo do jogador
        const float SpeedMultiplier = FMath::Max(1.0f, AttractionRadius / FMath::Max(Dist, 1.0f));
        SetActorLocation(GetActorLocation() + Dir * MoveSpeed * SpeedMultiplier * DeltaTime);
        
        // Adicione algum movimento de bobbing/flutua��o para efeito visual
        AddActorWorldOffset(FVector(0, 0, FMath::Sin(GetGameTimeSinceCreation() * 5.0f) * 0.5f));
        
        // Fa�a o orbe girar para efeito visual
        FRotator NewRotation = GetActorRotation();
        NewRotation.Yaw += 180.0f * DeltaTime;
        SetActorRotation(NewRotation);
    } else {
        // Mesmo quando longe do jogador, adicione um pequeno movimento de flutua��o
        AddActorWorldOffset(FVector(0, 0, FMath::Sin(GetGameTimeSinceCreation() * 2.0f) * 0.2f));
    }
}

void AXPOrb::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                       int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
    if (!OtherActor || OtherActor != TargetPlayer) return;
    if (UXPComponent* XP = OtherActor->FindComponentByClass<UXPComponent>()) {
        XP->AddXP(static_cast<float>(XPAmount));
    LOG_UPGRADES(Info, TEXT("XPOrb collected by player: +%d XP"), XPAmount);
        
        // Feedback visual antes de destruir (opcional)
        if (UWorld* World = GetWorld()) {
            // Aqui poderia spawnar um efeito de part�cula
        }
        
        Destroy();
    } else {
    LOG_UPGRADES(Warn, TEXT("Player doesn't have XPComponent!"));
    }
}
