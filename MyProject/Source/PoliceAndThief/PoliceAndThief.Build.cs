// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PoliceAndThief : ModuleRules
{
	public PoliceAndThief(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine",
			//Input
			"InputCore", "EnhancedInput",
			//AI
			"AIModule", "NavigationSystem",
			//UI
			"UMG",
		});
		PublicIncludePaths.AddRange(new string[] { "PoliceAndThief" });
	}
}
