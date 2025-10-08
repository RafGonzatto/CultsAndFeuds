#include "Enemy/Components/EnemyAuraComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/OverlapResult.h"
#include "Enemy/EnemyTypes.h"
#include "Logging/VazioLogFacade.h"

UEnemyAuraComponent::UEnemyAuraComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick 10 times per second for performance
    
    Radius = 400.f;
    DPS = 5.f;
    bAuraActive = true;
    DamageTickRate = 0.5f;
    DamageAccumulator = 0.f;
}

void UEnemyAuraComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UEnemyAuraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAuraActive)
    {
        DealAuraDamage(DeltaTime);
    }
}

void UEnemyAuraComponent::SetAuraProperties(float NewRadius, float NewDPS)
{
    Radius = NewRadius;
    DPS = NewDPS;
    
    LOG_ENEMIES(Trace, TEXT("Aura component configured: Radius=%.1f, DPS=%.1f"), Radius, DPS);
}

void UEnemyAuraComponent::DealAuraDamage(float DeltaTime)
{
    DamageAccumulator += DeltaTime;
    
    if (DamageAccumulator >= DamageTickRate)
    {
        float DamageThisTick = DPS * DamageAccumulator;
        DamageAccumulator = 0.f;
        
        TArray<AActor*> TargetsInRadius;
        FindTargetsInRadius(TargetsInRadius);
        
        for (AActor* Target : TargetsInRadius)
        {
            if (APawn* TargetPawn = Cast<APawn>(Target))
            {
                // Check if it's a player pawn
                if (TargetPawn->IsPlayerControlled())
                {
                    // Deal damage to player
                    // This would depend on your damage system implementation
                    LOG_LOOP_THROTTLE(Debug, TargetPawn->GetFName(), 1, DamageTickRate,
                        TEXT("Aura dealing %.1f damage to %s"), DamageThisTick, *TargetPawn->GetName());
                    
                    // Example: if you have a health component system
                    // if (UHealthComponent* HealthComp = TargetPawn->FindComponentByClass<UHealthComponent>())
                    // {
                    //     HealthComp->TakeDamage(DamageThisTick);
                    // }
                }
            }
        }
    }
}

void UEnemyAuraComponent::FindTargetsInRadius(TArray<AActor*>& OutTargets)
{
    if (!GetOwner())
    {
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Use overlap sphere to find targets
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHasOverlaps = GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        OwnerLocation,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );
    
    if (bHasOverlaps)
    {
        for (const FOverlapResult& Result : OverlapResults)
        {
            if (Result.GetActor() && Result.GetActor()->IsA<APawn>())
            {
                OutTargets.Add(Result.GetActor());
            }
        }
    }
}
