#include "Services/EnemyPerceptionService.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

void UEnemyPerceptionService::Initialize(UWorld* InWorld)
{
    CachedWorld = InWorld;
    UpdatePrimaryTarget();
}

void UEnemyPerceptionService::TickPerception(float DeltaTime)
{
    UpdatePrimaryTarget();
}

FVector UEnemyPerceptionService::GetPrimaryTargetLocation() const
{
    FScopeLock Lock(&TargetLock);
    return CachedPrimaryTargetLocation;
}

bool UEnemyPerceptionService::HasValidPrimaryTarget() const
{
    FScopeLock Lock(&TargetLock);
    return bHasPrimaryTarget;
}

void UEnemyPerceptionService::UpdatePrimaryTarget()
{
    UWorld* World = CachedWorld.Get();
    if (!World)
    {
        return;
    }

    FVector NewLocation = FVector::ZeroVector;
    bool bNewHasTarget = false;

    APawn* PrimaryPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (PrimaryPawn)
    {
        NewLocation = PrimaryPawn->GetActorLocation();
        bNewHasTarget = true;
    }

    FScopeLock Lock(&TargetLock);
    CachedPrimaryTargetLocation = NewLocation;
    bHasPrimaryTarget = bNewHasTarget;
}
