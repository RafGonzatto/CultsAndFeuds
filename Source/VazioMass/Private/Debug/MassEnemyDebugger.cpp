#include "Debug/MassEnemyDebugger.h"

#include "Visualization/EnemyMassVisualizationProcessor.h"

void UMassEnemyDebugger::DebugDrawEntities(UWorld* World)
{
    if (!World)
    {
        return;
    }

    GMassEnemyDebug.AsVariable()->Set(1);
}
