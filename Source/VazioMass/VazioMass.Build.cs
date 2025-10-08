using UnrealBuildTool;

public class VazioMass: ModuleRules
{
    public VazioMass(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "MassEntity",
            "MassCommon",
            "StructUtils",
            "VazioShared"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "MassActors",
            "MassMovement",
            "MassLOD",
            "MassSpawner",
            "MassNavigation",
            "MassSimulation",
            "DeveloperSettings",
            "AIModule",
            "NetCore",
            "Networking",
            "PacketHandler",
            "OnlineSubsystem"
        });
    }
}
