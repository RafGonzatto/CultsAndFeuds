using System.IO;
using UnrealBuildTool;

public class Vazio : ModuleRules
{
    public Vazio(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core","CoreUObject","Engine","InputCore","UMG","Slate","SlateCore","EnhancedInput","NavigationSystem",
            "OnlineSubsystem", "OnlineSubsystemUtils", "Json"
        });

        if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.Mac)
        {
            PrivateDependencyModuleNames.Add("OnlineSubsystemSteam");
        }

        ConfigureSteamSDK(Target);
    }

    private void ConfigureSteamSDK(ReadOnlyTargetRules Target)
    {
        string SteamSDKPath = Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "Steamworks"); // <ProjectRoot>/ThirdParty/Steamworks
        if (!Directory.Exists(SteamSDKPath))
        {
            PublicDefinitions.Add("STEAM_SDK_AVAILABLE=0");
            return;
        }

        // Common includes
        string IncludePath = Path.Combine(SteamSDKPath, "Include");
        if (Directory.Exists(IncludePath))
        {
            PublicIncludePaths.Add(IncludePath);
        }

        bool bLinked = false;

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            // Expect libs in a Win64 (or Lib) folder – adapt to your layout
            string LibFolder = Directory.Exists(Path.Combine(SteamSDKPath, "Win64"))
                ? Path.Combine(SteamSDKPath, "Win64")
                : Path.Combine(SteamSDKPath, "Lib");

            string LibFile = Path.Combine(LibFolder, "steam_api64.lib");
            string DllFile = Path.Combine(LibFolder, "steam_api64.dll");

            if (File.Exists(LibFile) && File.Exists(DllFile))
            {
                PublicAdditionalLibraries.Add(LibFile);
                PublicDelayLoadDLLs.Add("steam_api64.dll");
                RuntimeDependencies.Add("$(TargetOutputDir)/steam_api64.dll", DllFile);
                bLinked = true;
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            string LibFile = Path.Combine(SteamSDKPath, "linux64", "libsteam_api.so");
            if (File.Exists(LibFile))
            {
                PublicAdditionalLibraries.Add(LibFile);
                RuntimeDependencies.Add(LibFile);
                bLinked = true;
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            string LibFile = Path.Combine(SteamSDKPath, "osx", "libsteam_api.dylib");
            if (File.Exists(LibFile))
            {
                PublicAdditionalLibraries.Add(LibFile);
                RuntimeDependencies.Add(LibFile);
                bLinked = true;
            }
        }

        PublicDefinitions.Add(bLinked ? "STEAM_SDK_AVAILABLE=1" : "STEAM_SDK_AVAILABLE=0");
    }
}
