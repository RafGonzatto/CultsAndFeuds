using System.IO;
using UnrealBuildTool;

public class Vazio : ModuleRules
{
    public Vazio(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core","CoreUObject","Engine","InputCore","UMG","Slate","SlateCore","EnhancedInput","NavigationSystem",
            "OnlineSubsystem", "OnlineSubsystemUtils"
        });

        // Add Steam if available
        if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.Mac)
        {
            PrivateDependencyModuleNames.Add("OnlineSubsystemSteam");
        }
      
    }
}
