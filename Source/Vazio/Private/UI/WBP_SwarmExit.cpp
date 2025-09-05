#include "UI/WBP_SwarmExit.h"
#include "Swarm/SwarmGameFlow.h"
#include "Kismet/GameplayStatics.h"

void UWBP_SwarmExit::OnExitClicked()
{
    if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
    {
        if (USwarmGameFlow* Flow = GI->GetSubsystem<USwarmGameFlow>())
        {
            Flow->ExitArena();
        }
    }
}

