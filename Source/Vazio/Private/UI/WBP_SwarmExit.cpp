#include "UI/WBP_SwarmExit.h"
// #include "Swarm/SwarmGameFlow.h" // Removed - Swarm system deleted
#include "Kismet/GameplayStatics.h"

void UWBP_SwarmExit::OnExitClicked()
{
    // TODO: SwarmGameFlow removed - implement generic exit logic
    UE_LOG(LogTemp, Warning, TEXT("[WBP_SwarmExit] Exit clicked - SwarmGameFlow removed, no action taken"));
}

