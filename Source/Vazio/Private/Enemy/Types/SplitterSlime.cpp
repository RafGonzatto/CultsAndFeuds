#include "Enemy/Types/SplitterSlime.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy/EnemySpawnerSubsystem.h"
#include "Enemy/EnemyTypes.h"

ASplitterSlime::ASplitterSlime()
{
    PrimaryActorTick.bCanEverTick = true;
    PursuitSpeed = 200.f;
    StoppingDistance = 100.f;
    ChildrenCount = 2;
    ChildrenSpawnRadius = 100.f;
    ChildrenHPMultiplier = 0.5f;
    ChildrenDMGMultiplier = 0.5f;
}

void ASplitterSlime::BeginPlay()
{
    Super::BeginPlay();
}

void ASplitterSlime::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!CurrentModifiers.bImmovable)
    {
        PursuePlayer(DeltaTime);
    }
}

void ASplitterSlime::HandleDeath(bool bIsParentParam)
{
    // If this is a parent slime, create children before dying
    if (bIsParentParam)
    {
        CreateChildren();
    }
    
    // Call parent death handling
    Super::HandleDeath(bIsParentParam);
}

void ASplitterSlime::PursuePlayer(float DeltaTime)
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

void ASplitterSlime::CreateChildren()
{
    UEnemySpawnerSubsystem* SpawnerSubsystem = GetWorld()->GetSubsystem<UEnemySpawnerSubsystem>();
    if (!SpawnerSubsystem)
    {
        UE_LOG(LogEnemy, Warning, TEXT("SplitterSlime: Could not find EnemySpawnerSubsystem"));
        return;
    }

    FVector ParentLocation = GetActorLocation();
    
    // Create modified archetype for children
    FEnemyArchetype ChildArchetype = CurrentArchetype;
    ChildArchetype.BaseHP *= ChildrenHPMultiplier;
    ChildArchetype.BaseDMG *= ChildrenDMGMultiplier;
    
    // Children don't split further and should have normal drop behavior
    ChildArchetype.Death = EOnDeathBehavior::Normal;
    
    for (int32 i = 0; i < ChildrenCount; i++)
    {
        // Calculate spawn position around parent
        float Angle = (2.0f * PI * i) / ChildrenCount;
        FVector Offset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * ChildrenSpawnRadius;
        FVector ChildLocation = ParentLocation + Offset;
        
        // Spawn child slime
        FTransform ChildTransform;
        ChildTransform.SetLocation(ChildLocation);
        ChildTransform.SetRotation(GetActorQuat());
        ChildTransform.SetScale3D(GetActorScale3D() * 0.8f); // Slightly smaller children
        
        FEnemyInstanceModifiers ChildMods = CurrentModifiers;
        // Children inherit modifiers but are not parents
        
        if (ASplitterSlime* Child = Cast<ASplitterSlime>(SpawnerSubsystem->SpawnOne(TEXT("SplitterSlime"), ChildTransform, ChildMods)))
        {
            Child->bIsParent = false; // Children don't split
            Child->ApplyArchetypeAndModifiers(ChildArchetype, ChildMods);
            
            UE_LOG(LogEnemy, Log, TEXT("SplitterSlime parent created child %d at %s"), i, *ChildLocation.ToCompactString());
        }
    }
    
    UE_LOG(LogEnemy, Log, TEXT("SplitterSlime parent created %d children"), ChildrenCount);
}
