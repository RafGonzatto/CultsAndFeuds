#include "UI/WBP_SwarmExit.h"
// #include "Swarm/SwarmGameFlow.h" // Removed - Swarm system deleted
#include "Kismet/GameplayStatics.h"
#include "Logging/VazioLogFacade.h"

void UWBP_SwarmExit::OnExitClicked()
{
    // TODO: SwarmGameFlow removed - implement generic exit logic
    LOG_UI(Warn, TEXT("[WBP_SwarmExit] Exit clicked - SwarmGameFlow removed, no action taken"));
}

