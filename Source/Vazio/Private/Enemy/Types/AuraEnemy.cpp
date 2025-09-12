#include "Enemy/Types/AuraEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy/EnemyTypes.h"

AAuraEnemy::AAuraEnemy()
{
    PrimaryActorTick.bCanEverTick = true;
    PursuitSpeed = 250.f;
    StoppingDistance = 300.f;
}

void AAuraEnemy::BeginPlay()
{
    Super::BeginPlay();
}

void AAuraEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!CurrentModifiers.bImmovable)
    {
        PursuePlayer(DeltaTime);
    }
}

void AAuraEnemy::PursuePlayer(float DeltaTime)
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
        
        // Move towards player
        AddMovementInput(Direction, 1.0f);
        
        // Face the player
        SetActorRotation(FRotationMatrix::MakeFromX(Direction).Rotator());
    }
    else
    {
        // In aura range, just face the player
        FVector Direction = ToPlayer.GetSafeNormal();
        SetActorRotation(FRotationMatrix::MakeFromX(Direction).Rotator());
    }
}
