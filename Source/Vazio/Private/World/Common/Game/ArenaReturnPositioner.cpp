#include "World/Common/Game/ArenaReturnPositioner.h"
#include "Swarm/SwarmGameFlow.h"
#include "Kismet/GameplayStatics.h"

void AArenaReturnPositioner::BeginPlay()
{
    Super::BeginPlay();
    UWorld* World = GetWorld();
    if (!World) return;
    if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
    {
        if (USwarmGameFlow* Flow = GI->GetSubsystem<USwarmGameFlow>())
        {
            const FName CurrentLevel = FName(*UGameplayStatics::GetCurrentLevelName(this, true));
            if (!Flow->GetReturnLevelName().IsNone() && Flow->GetReturnLevelName() == CurrentLevel)
            {
                if (APlayerController* PC = World->GetFirstPlayerController())
                {
                    if (APawn* Pawn = PC->GetPawn())
                    {
                        const FTransform Xf = Flow->GetReturnTransform();
                        if (!Xf.GetLocation().IsNearlyZero())
                        {
                            Pawn->SetActorTransform(Xf);
                            UE_LOG(LogTemp, Warning, TEXT("[ArenaReturnPositioner] Player moved to return: %s"), *Xf.GetLocation().ToString());
                        }
                    }
                }
            }
        }
    }
}

