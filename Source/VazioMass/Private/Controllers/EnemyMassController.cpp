#include "Controllers/EnemyMassController.h"

#include "MassEntitySubsystem.h"
#include "MassCommandBuffer.h"
#include "Services/EnemyMovementService.h"
#include "Services/EnemyPerceptionService.h"
#include "Services/EnemyAIService.h"
#include "Engine/World.h"

void UEnemyMassController::Initialize(UWorld* InWorld, UMassEntitySubsystem* InEntitySubsystem, const TSharedPtr<FMassCommandBuffer>& InCommandBuffer)
{
    CachedWorld = InWorld;
    CachedEntitySubsystem = InEntitySubsystem;
    CommandBuffer = InCommandBuffer;

    InitializeServices();
    StartGameThreadTicker();
}

void UEnemyMassController::InitializeServices()
{
    if (!MovementService)
    {
        MovementService = NewObject<UEnemyMovementService>(this);
    }

    if (MovementService)
    {
        MovementService->Initialize(1200.0f, 150.0f, 400.0f);
    }

    if (!PerceptionService)
    {
        PerceptionService = NewObject<UEnemyPerceptionService>(this);
    }

    if (PerceptionService)
    {
        PerceptionService->Initialize(CachedWorld.Get());
    }

    if (!AIService)
    {
        AIService = NewObject<UEnemyAIService>(this);
    }

    if (AIService)
    {
        AIService->Initialize(450.0f, 220.0f, 350.0f);
    }

    // TODO(DEPRECATE): Damage service disabled for initial Mass bring-up
}

void UEnemyMassController::StartGameThreadTicker()
{
    StopGameThreadTicker();

    if (!CachedWorld.IsValid())
    {
        return;
    }

    TickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this, [this](float DeltaTime)
    {
        if (PerceptionService)
        {
            PerceptionService->TickPerception(DeltaTime);
        }

        // TODO(DEPRECATE): Command buffer flush disabled while damage service is off

        return true;
    }));
}

void UEnemyMassController::StopGameThreadTicker()
{
    if (TickerHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
        TickerHandle = FTSTicker::FDelegateHandle();
    }
}

// Damage queue disabled in bring-up
