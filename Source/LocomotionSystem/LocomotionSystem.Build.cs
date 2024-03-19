// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LocomotionSystem : ModuleRules
{
    public LocomotionSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicIncludePaths.Add("LocomotionSystem");

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
    }
}
