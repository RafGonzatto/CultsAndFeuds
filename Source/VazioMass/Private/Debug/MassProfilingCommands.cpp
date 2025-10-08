#include "Debug/MassProfilingCommands.h"

#include "Enemy/EnemyTypes.h"
#include "MassEntitySubsystem.h"
#include "Systems/EnemyMassSpawnerSubsystem.h"
#include "Visualization/EnemyMassVisualizationProcessor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Logging/VazioLogFacade.h"

namespace
{
    IConsoleObject* GSpawnTestHordeCmd = nullptr;
    IConsoleObject* GToggleVisCmd = nullptr;
    IConsoleObject* GDumpStatsCmd = nullptr;
    IConsoleObject* GToggleOverlayCmd = nullptr;

    UWorld* GetAnyWorld()
    {
        if (GEngine)
        {
            if (const FWorldContext* WorldContext = (GEngine->GetWorldContexts().Num() > 0) ? &GEngine->GetWorldContexts()[0] : nullptr)
            {
                return WorldContext->World();
            }
        }

        return nullptr;
    }

    void SpawnTestHorde(const TArray<FString>& Args)
    {
        UWorld* World = GetAnyWorld();
        if (!World)
        {
            LOG_MASS(Warn, TEXT("mass.SpawnTestHorde: No world available."));
            return;
        }

        UEnemyMassSpawnerSubsystem* MassSpawner = World->GetSubsystem<UEnemyMassSpawnerSubsystem>();
        if (!MassSpawner)
        {
            LOG_MASS(Warn, TEXT("mass.SpawnTestHorde: Mass spawner subsystem unavailable."));
            return;
        }

        const FName ArchetypeName = Args.Num() > 0 ? FName(*Args[0]) : FName(TEXT("NormalEnemy"));
        const int32 Count = Args.Num() > 1 ? FCString::Atoi(*Args[1]) : 25;

        MassSpawner->SpawnEnemies(ArchetypeName, Count, FVector::ZeroVector, FEnemyInstanceModifiers());
        LOG_MASS(Info, TEXT("Spawned %d mass enemies of type %s for profiling."), Count, *ArchetypeName.ToString());
    }

    void ToggleEntityVisualization()
    {
        const int32 Current = GMassEnemyDebug.GetValueOnGameThread();
        GMassEnemyDebug.AsVariable()->Set(Current == 0 ? 1 : 0);
    }

    void DumpEntityStats()
    {
        UWorld* World = GetAnyWorld();
        if (!World)
        {
            return;
        }

        // TODO: Implement precise count using FMassEntityManager API compatible with UE 5.6
        LOG_MASS(Info, TEXT("Mass enemy entity stats dump placeholder (implement with UE 5.6 API)"));
    }

    void TogglePerformanceOverlay()
    {
        ToggleEntityVisualization();
    }
}

namespace FMassProfilingCommands
{
    void Register()
    {
        IConsoleManager& ConsoleManager = IConsoleManager::Get();

        GSpawnTestHordeCmd = ConsoleManager.RegisterConsoleCommand(
            TEXT("mass.SpawnTestHorde"),
            TEXT("Spawns a test horde of mass enemies to benchmark performance"),
            FConsoleCommandWithArgsDelegate::CreateStatic(&SpawnTestHorde));

        GToggleVisCmd = ConsoleManager.RegisterConsoleCommand(
            TEXT("mass.ToggleEntityVisualization"),
            TEXT("Toggles debug visualization of mass enemies"),
            FConsoleCommandDelegate::CreateStatic(&ToggleEntityVisualization));

        GDumpStatsCmd = ConsoleManager.RegisterConsoleCommand(
            TEXT("mass.DumpEntityStats"),
            TEXT("Dumps statistics about mass enemies to the log"),
            FConsoleCommandDelegate::CreateStatic(&DumpEntityStats));

        GToggleOverlayCmd = ConsoleManager.RegisterConsoleCommand(
            TEXT("mass.TogglePerformanceOverlay"),
            TEXT("Toggles the mass performance overlay"),
            FConsoleCommandDelegate::CreateStatic(&TogglePerformanceOverlay));
    }

    void Unregister()
    {
        IConsoleManager& ConsoleManager = IConsoleManager::Get();
        if (GSpawnTestHordeCmd)
        {
            ConsoleManager.UnregisterConsoleObject(GSpawnTestHordeCmd, false);
            GSpawnTestHordeCmd = nullptr;
        }

        if (GToggleVisCmd)
        {
            ConsoleManager.UnregisterConsoleObject(GToggleVisCmd, false);
            GToggleVisCmd = nullptr;
        }

        if (GDumpStatsCmd)
        {
            ConsoleManager.UnregisterConsoleObject(GDumpStatsCmd, false);
            GDumpStatsCmd = nullptr;
        }

        if (GToggleOverlayCmd)
        {
            ConsoleManager.UnregisterConsoleObject(GToggleOverlayCmd, false);
            GToggleOverlayCmd = nullptr;
        }
    }
}
