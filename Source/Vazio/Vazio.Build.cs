using UnrealBuildTool;

public class Vazio : ModuleRules
{
    public Vazio(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core","CoreUObject","Engine","InputCore","UMG","Slate","SlateCore"
        });
    }
}
