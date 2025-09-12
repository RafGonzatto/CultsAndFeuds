#include "Enemy/Types/HeavyEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy/EnemyTypes.h"

AHeavyEnemy::AHeavyEnemy()
{
    PrimaryActorTick.bCanEverTick = true;
    HeavyPursuitSpeed = 150.f;
    StoppingDistance = 120.f;
    
    // Heavy enemies are tankier
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->MaxWalkSpeed = HeavyPursuitSpeed;
    }
}

void AHeavyEnemy::BeginPlay()
{
    Super::BeginPlay();
}

void AHeavyEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!CurrentModifiers.bImmovable)
    {
        PursuePlayer(DeltaTime);
    }
}

void AHeavyEnemy::PursuePlayer(float DeltaTime)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
    float DistanceToPlayer = ToPlayer.Size();

    if (DistanceToPlayer > StoppingDistance)
    {
        FVector Direction = ToPlayer.GetSafeNormal();
        
        // Move towards player (slower than normal enemy)
        AddMovementInput(Direction, 1.0f);
        
        // Face the player
        FRotator TargetRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
        SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 2.0f));
    }
}
