#include "Enemy/Types/NormalEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy/EnemyTypes.h"

ANormalEnemy::ANormalEnemy()
{
    PrimaryActorTick.bCanEverTick = true;
    PursuitSpeed = 300.f;
    StoppingDistance = 100.f;
}

void ANormalEnemy::BeginPlay()
{
    Super::BeginPlay();
}

void ANormalEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!CurrentModifiers.bImmovable)
    {
        PursuePlayer(DeltaTime);
    }
}

void ANormalEnemy::PursuePlayer(float DeltaTime)
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
}
