#include "Enemy/Types/DashEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Enemy/EnemyTypes.h"

ADashEnemy::ADashEnemy()
{
    PrimaryActorTick.bCanEverTick = true;
    DashSpeed = 1000.f;
    DashDuration = 0.5f;
    MinDashDistance = 200.f;
    bIsDashing = false;
    bCanDash = true;
    DashTimeRemaining = 0.f;
}

void ADashEnemy::BeginPlay()
{
    Super::BeginPlay();
}

void ADashEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!CurrentModifiers.bImmovable)
    {
        HandleDashBehavior(DeltaTime);
    }
}

void ADashEnemy::HandleDashBehavior(float DeltaTime)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
    float DistanceToPlayer = ToPlayer.Size();
    FVector Direction = ToPlayer.GetSafeNormal();

    if (bIsDashing)
    {
        // Continue dashing
        DashTimeRemaining -= DeltaTime;
        
        if (DashTimeRemaining > 0.f)
        {
            // Move in dash direction at high speed
            AddMovementInput(DashDirection, DashSpeed / GetCharacterMovement()->MaxWalkSpeed);
        }
        else
        {
            // Dash completed
            bIsDashing = false;
            bCanDash = false;
            
            // Start cooldown
            GetWorld()->GetTimerManager().SetTimer(
                DashCooldownTimerHandle,
                this,
                &ADashEnemy::OnDashCooldownComplete,
                CurrentArchetype.DashCooldown,
                false
            );
            
            UE_LOG(LogEnemy, VeryVerbose, TEXT("%s completed dash, cooldown started"), *GetName());
        }
    }
    else
    {
        // Normal movement and dash logic
        if (DistanceToPlayer > 150.f)
        {
            // Move towards player normally
            AddMovementInput(Direction, 1.0f);
        }
        
        // Face the player
        SetActorRotation(FRotationMatrix::MakeFromX(Direction).Rotator());
        
        // Check if we should dash
        if (bCanDash && CurrentArchetype.bCanDash && DistanceToPlayer >= MinDashDistance && DistanceToPlayer <= CurrentArchetype.DashDistance * 1.5f)
        {
            ExecuteDash();
        }
    }
}

void ADashEnemy::ExecuteDash()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
    DashDirection = ToPlayer.GetSafeNormal();
    DashTimeRemaining = DashDuration;
    bIsDashing = true;
    
    UE_LOG(LogEnemy, Log, TEXT("%s executing dash towards player"), *GetName());
}

void ADashEnemy::OnDashCooldownComplete()
{
    bCanDash = true;
    UE_LOG(LogEnemy, VeryVerbose, TEXT("%s dash cooldown complete"), *GetName());
}
