#include "Enemy/Types/SplitterSlime.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy/EnemySpawnerSubsystem.h"
#include "Enemy/EnemyTypes.h"
#include "Systems/EnemyMassSpawnerSubsystem.h"
#include "Systems/EnemyImplementationCVars.h"
#include "Logging/VazioLogFacade.h"

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
    if (GetEnemyImpl() == 1)
    {
        if (UWorld* World = GetWorld())
        {
            if (UEnemyMassSpawnerSubsystem* MassSpawner = World->GetSubsystem<UEnemyMassSpawnerSubsystem>())
            {
                const FVector ParentLocation = GetActorLocation();

                FEnemyInstanceModifiers ChildMods = CurrentModifiers;
                ChildMods.HealthMultiplier *= ChildrenHPMultiplier;
                ChildMods.DamageMultiplier *= ChildrenDMGMultiplier;
                ChildMods.ScaleMultiplier *= 0.8f;

                TArray<FTransform> SpawnTransforms;
                SpawnTransforms.Reserve(ChildrenCount);

                for (int32 Index = 0; Index < ChildrenCount; ++Index)
                {
                    const float Angle = (2.0f * PI * Index) / FMath::Max(1, ChildrenCount);
                    const FVector Offset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * ChildrenSpawnRadius;
                    const FVector ChildLocation = ParentLocation + Offset;
                    SpawnTransforms.Emplace(GetActorQuat(), ChildLocation, GetActorScale3D() * 0.8f);
                }

                MassSpawner->SpawnEnemiesAtTransforms(TEXT("SplitterSlime"), SpawnTransforms, ChildMods);
                LOG_ENEMIES(Info, TEXT("SplitterSlime parent created %d children via Mass"), ChildrenCount);
                return;
            }
        }

    LOG_ENEMIES(Warn, TEXT("SplitterSlime: Mass spawner unavailable, skipping child creation"));
        return;
    }

    UEnemySpawnerSubsystem* SpawnerSubsystem = GetWorld()->GetSubsystem<UEnemySpawnerSubsystem>();
    if (!SpawnerSubsystem)
    {
    LOG_ENEMIES(Warn, TEXT("SplitterSlime: Could not find EnemySpawnerSubsystem"));
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
            
            LOG_ENEMIES(Info, TEXT("SplitterSlime parent created child %d at %s"), i, *ChildLocation.ToCompactString());
        }
    }
    
    LOG_ENEMIES(Info, TEXT("SplitterSlime parent created %d children"), ChildrenCount);
}
