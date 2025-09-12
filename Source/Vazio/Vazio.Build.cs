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

        // Add Steam if available
        if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.Mac)
        {
            PrivateDependencyModuleNames.Add("OnlineSubsystemSteam");
        }

        // Steam SDK Integration
        SetupSteamSDK(Target);
    }

    private void SetupSteamSDK(ReadOnlyTargetRules Target)
    {
        string SteamSDKPath = Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "Steamworks");
        
        if (Directory.Exists(SteamSDKPath))
        {
            string IncludePath = Path.Combine(SteamSDKPath, "Include");
            string LibPath = Path.Combine(SteamSDKPath, "Lib");
            
            if (Directory.Exists(IncludePath))
            {
                PublicIncludePaths.Add(IncludePath);
                PrivateIncludePaths.Add(IncludePath);
            }
            
            if (Directory.Exists(LibPath) && Target.Platform == UnrealTargetPlatform.Win64)
            {
                PublicAdditionalLibraries.Add(Path.Combine(LibPath, "steam_api64.lib"));
                PublicDelayLoadDLLs.Add("steam_api64.dll");
                
                // Copy DLL to Binaries folder during build
                string BinariesPath = Path.Combine(ModuleDirectory, "..", "..", "Binaries", "Win64");
                if (!Directory.Exists(BinariesPath))
                {
                    Directory.CreateDirectory(BinariesPath);
                }
                
                string SourceDLL = Path.Combine(LibPath, "steam_api64.dll");
                string TargetDLL = Path.Combine(BinariesPath, "steam_api64.dll");
                
                if (File.Exists(SourceDLL))
                {
                    try
                    {
                        File.Copy(SourceDLL, TargetDLL, true);
                    }
                    catch (System.Exception)
                    {
                        // Silent fail - DLL might be in use
                    }
                }
                
                PublicDefinitions.Add("STEAM_SDK_AVAILABLE=1");
            }
        }
        else
        {
            PublicDefinitions.Add("STEAM_SDK_AVAILABLE=0");
        }
    }
}
