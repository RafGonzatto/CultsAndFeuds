#include "Swarm/SwarmGameFlow.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void USwarmGameFlow::EnterArena(FName BattleLevel)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[SwarmGameFlow] EnterArena: World is null"));
        return;
    }

    // Always capture the current level as return level if not explicitly set elsewhere
    const FName CurrentLevelName = FName(*UGameplayStatics::GetCurrentLevelName(World, true));
    if (ReturnLevel.IsNone())
    {
        ReturnLevel = CurrentLevelName;
    }

    // Force Battle map to use our battle GameMode to guarantee pawn/controller/playerstart
    const FString Options = TEXT("game=/Script/Vazio.BattleGameMode");
    UE_LOG(LogTemp, Display, TEXT("[SwarmGameFlow] EnterArena -> OpenLevel %s (return to %s)"), *BattleLevel.ToString(), *ReturnLevel.ToString());
    UGameplayStatics::OpenLevel(World, BattleLevel, true, Options);
}

void USwarmGameFlow::ExitArena()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[SwarmGameFlow] ExitArena: World is null"));
        return;
    }

    const FName Target = ReturnLevel.IsNone() ? FName(*UGameplayStatics::GetCurrentLevelName(World, true)) : ReturnLevel;
    UE_LOG(LogTemp, Display, TEXT("[SwarmGameFlow] ExitArena -> OpenLevel %s (will reposition on City BeginPlay)"), *Target.ToString());
    UGameplayStatics::OpenLevel(World, Target);
}

