#include "Enemy/Types/GoldEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy/EnemyTypes.h"

AGoldEnemy::AGoldEnemy()
{
    PrimaryActorTick.bCanEverTick = true;
    PursuitSpeed = 210.f; // 70% of normal speed (300 * 0.7)
    StoppingDistance = 100.f;
    
    // Gold enemies are slower
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->MaxWalkSpeed = PursuitSpeed;
    }
}

void AGoldEnemy::BeginPlay()
{
    Super::BeginPlay();
}

void AGoldEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!CurrentModifiers.bImmovable)
    {
        PursuePlayer(DeltaTime);
    }
}

void AGoldEnemy::PursuePlayer(float DeltaTime)
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
        
        // Move towards player (slowly)
        AddMovementInput(Direction, 1.0f);
        
        // Face the player
        SetActorRotation(FRotationMatrix::MakeFromX(Direction).Rotator());
    }
}
