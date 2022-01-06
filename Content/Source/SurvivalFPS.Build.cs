// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SurvivalFPS : ModuleRules
{
	public SurvivalFPS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "AIModule", "Voxel", "MercunaCore", "Mercuna3DNavigation", "Mercuna2DNavigation", "Mercuna3DMovement", "Mercuna", "Voronoi", "OnlineSubsystem", "OnlineSubsystemSteam", "OnlineSubsystemUtils", "AnimGraph", "AnimGraphRuntime", "BlueprintGraph", "DynamicMesh", "ProceduralMeshComponent"});
		
		PrivateDependencyModuleNames.AddRange(new string[] {"MoveIt", "CableComponent"});
	}
}
