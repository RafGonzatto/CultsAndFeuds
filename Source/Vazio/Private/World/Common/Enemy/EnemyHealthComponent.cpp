#include "World/Common/Enemy/EnemyHealthComponent.h"
#include "World/Common/Collectables/XPOrb.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogEnemyHealth, Log, All);

UEnemyHealthComponent::UEnemyHealthComponent() {
    PrimaryComponentTick.bCanEverTick = false;
    CurrentHealth = MaxHealth;
}

void UEnemyHealthComponent::BeginPlay() {
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
    
    // Make sure the owner has proper collision settings
    AActor* Owner = GetOwner();
    if (Owner)
    {
        // Find and update all primitive components for collision detection
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Owner->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp)
            {
                // Ensure collision is enabled and this component can be detected by overlap queries
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                PrimComp->SetCollisionObjectType(ECC_Pawn); // Use Pawn type for better detection
                PrimComp->SetGenerateOverlapEvents(true);
            }
        }
    }
    
    UE_LOG(LogEnemyHealth, Display, TEXT("Enemy %s initialized with HP=%.1f"), *GetOwner()->GetName(), CurrentHealth);
}

void UEnemyHealthComponent::ReceiveDamage(float DamageAmount) {
    if (DamageAmount <= 0.f || CurrentHealth <= 0.f) return;

    CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.f, MaxHealth);
    UE_LOG(LogEnemyHealth, Display, TEXT("%s took %.1f damage → HP=%.1f"), *GetOwner()->GetName(), DamageAmount, CurrentHealth);

    // Visual indicator of damage
    if (AActor* Owner = GetOwner())
    {
        #if !(UE_BUILD_SHIPPING)
        DrawDebugSphere(GetWorld(), Owner->GetActorLocation(), 50.0f, 8, FColor::Red, false, 0.5f, 0, 2.0f);
        #endif
    }

    if (CurrentHealth <= 0.f) {
        UE_LOG(LogEnemyHealth, Warning, TEXT("%s died, dropping XPOrb with %d XP"), *GetOwner()->GetName(), XPValue);
        if (UWorld* World = GetWorld()) {
            const FVector L = GetOwner()->GetActorLocation();
            // Eleva a posição um pouco para evitar que o orb fique embaixo do terreno
            const FVector SpawnLocation = L + FVector(0, 0, 50.0f);
            FActorSpawnParameters P;
            P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            AXPOrb* Orb = World->SpawnActor<AXPOrb>(AXPOrb::StaticClass(), SpawnLocation, FRotator::ZeroRotator, P);
            if (Orb) {
                Orb->XPAmount = XPValue;
                UE_LOG(LogEnemyHealth, Display, TEXT("XPOrb spawned successfully at %s"), *SpawnLocation.ToString());
            } else {
                UE_LOG(LogEnemyHealth, Error, TEXT("Failed to spawn XPOrb!"));
            }
        }
        if (AActor* Owner = GetOwner()) {
            Owner->Destroy();
        }
    }
}
