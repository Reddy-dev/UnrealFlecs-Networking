// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UnrealFlecsNetworking : ModuleRules
{
	public UnrealFlecsNetworking(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		CppStandard = CppStandardVersion.Cpp23;
		IWYUSupport = IWYUSupport.Full;

		SetupIrisSupport(Target, true);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"FlecsLibrary",
				"SolidMacros",
				"UnrealFlecs",
				"NetCore",
			}
		);

		CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Error;
	}
}
