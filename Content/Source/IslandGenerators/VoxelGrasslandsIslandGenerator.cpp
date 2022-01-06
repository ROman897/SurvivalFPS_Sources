// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelGrasslandsIslandGenerator.h"
// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelMaterialBuilder.h"
#include "FastNoise/VoxelFastNoise.inl"
#include "SurvivalFPS/IslandsWorldManager.h"
#include "SurvivalFPS/SurvivalFPSMathFunctions.h"
#include "SurvivalFPS/VoxelIslandWorld.h"

DECLARE_STATS_GROUP(TEXT("GrasslandGenerator"), STATGROUP_GrasslandGenerator, STATCAT_Advanced);

/* Declaring a cycle stat that belongs to "Orfeas", named Orfeas-FindPrimeNumbers*/
DECLARE_CYCLE_STAT(TEXT("GrasslandGenerator - border noises"), STAT_BorderNoises, STATGROUP_GrasslandGenerator);
DECLARE_CYCLE_STAT(TEXT("GrasslandGenerator - border transformation"), STAT_BorderTransformation, STATGROUP_GrasslandGenerator);
DECLARE_CYCLE_STAT(TEXT("GrasslandGenerator - height noises"), STAT_HeightNoises, STATGROUP_GrasslandGenerator);

TVoxelSharedRef<FVoxelGeneratorInstance> UVoxelGrasslandsIslandGenerator::GetInstance()
{
	return MakeVoxelShared<FVoxelGrasslandsIslandGeneratorInstance>(*this);
}

FVoxelGrasslandsIslandGeneratorInstance::FVoxelGrasslandsIslandGeneratorInstance(const UVoxelGrasslandsIslandGenerator& MyGenerator)
	: Super(&MyGenerator)
	, TopNoiseHeight(MyGenerator.TopNoiseHeight)
	, TopNoiseFrequency(MyGenerator.TopNoiseFrequency)
	, TopNoiseOctaves(MyGenerator.TopNoiseOctaves)
	, BottomNoiseHeight(MyGenerator.BottomNoiseHeight)
	, BottomNoiseFrequency(MyGenerator.BottomNoiseFrequency)
	, BottomNoiseOctaves(MyGenerator.BottomNoiseOctaves)
	, BordersLargeFrequency(MyGenerator.BordersLargeFrequency)
	, BordersLargeOctaves(MyGenerator.BordersLargeOctaves)
	, BordersLargePercentageOfRadius(MyGenerator.BordersLargePercentageOfRadius)
	, BordersSmallFrequency(MyGenerator.BordersSmallFrequency)
	, BordersSmallOctaves(MyGenerator.BordersSmallOctaves)
	, BordersSmallNoiseHeight(MyGenerator.BordersSmallNoiseHeight)
	, VerticalNoiseFrequency(MyGenerator.VerticalNoiseFrequency)
	, VerticalNoiseOctaves(MyGenerator.VerticalNoiseOctaves)
	, VerticalNoiseHeight(MyGenerator.VerticalNoiseHeight)
	, MaterialIndex(MyGenerator.MaterialIndex)
	, IslandMinThickness(MyGenerator.IslandMinThickness)
{
}

void FVoxelGrasslandsIslandGeneratorInstance::Init(const FVoxelGeneratorInit& InitStruct)
{
	Super::Init(InitStruct);

	const AVoxelIslandWorld* VoxelIslandWorld = Cast<AVoxelIslandWorld>(InitStruct.World);
	IslandRadiusInVoxels = VoxelIslandWorld->IslandRadiusInVoxels;
	check(IslandRadiusInVoxels > 0);
	
	FRandomStream& WorldRandomStream = AIslandsWorldManager::GetInstance()->GetWorldRandomStream();
	TopNoise.SetSeed(WorldRandomStream.RandRange(0, TNumericLimits<int32>::Max() - 1));
	BottomNoise.SetSeed(WorldRandomStream.RandRange(0, TNumericLimits<int32>::Max() - 1));
	BordersLargeNoise.SetSeed(WorldRandomStream.RandRange(0, TNumericLimits<int32>::Max() - 1));
	BordersSmallNoise.SetSeed(WorldRandomStream.RandRange(0, TNumericLimits<int32>::Max() - 1));
	VerticalNoise.SetSeed(WorldRandomStream.RandRange(0, TNumericLimits<int32>::Max() - 1));
}

v_flt FVoxelGrasslandsIslandGeneratorInstance::GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD,
                                                        const FVoxelItemStack& Items) const
{
	const float MaxHeight = TopNoiseHeight + IslandMinThickness * 0.5f;
	if (FMath::Abs(Z) > MaxHeight)
	{
		return 1;
	}
	
	float HeightVal = TopNoise.GetSimplex_2D(X, Y, TopNoiseFrequency) * TopNoiseHeight + IslandMinThickness * 0.5f;
	if (FMath::Abs(Z) > HeightVal + 4)
	{
		return 1.0f;
	}
	
	float ActualDist = FVector2D(X, Y).Size();

	if (ActualDist > IslandRadiusInVoxels)
	{
		return 1;
	}

	float BordersLargeNoiseVal = FMath::Abs(BordersLargeNoise.GetSimplexFractal_2D(X, Y, BordersLargeFrequency, BordersLargeOctaves) * IslandRadiusInVoxels * BordersLargePercentageOfRadius);

	float MinDistWithGivenLargeNoise = ActualDist + BordersLargeNoiseVal - BordersSmallNoiseHeight - VerticalNoiseHeight;
	if (MinDistWithGivenLargeNoise > IslandRadiusInVoxels)
	{
		return 1;
	}
	
	float BordersSmallNoiseVal = FMath::Abs(BordersSmallNoise.GetSimplexFractal_2D(X, Y, BordersSmallFrequency, BordersSmallOctaves) * BordersSmallNoiseHeight);

	float MinDistGivenLargeAndSmallNoise = ActualDist + BordersLargeNoiseVal + BordersSmallNoiseVal - VerticalNoiseHeight;
	if (MinDistGivenLargeAndSmallNoise > IslandRadiusInVoxels)
	{
		return 1;
	}
	
	float VerticalNoiseVal = FMath::Abs(VerticalNoise.GetSimplexFractal_3D(X, Y, Z, VerticalNoiseFrequency, VerticalNoiseOctaves) * VerticalNoiseHeight);
	
	float DistDiff = ActualDist - IslandRadiusInVoxels + BordersLargeNoiseVal + BordersSmallNoiseVal + VerticalNoiseVal;

	if (DistDiff > 0.0f)
	{
		return 1;
	}

	if (Z >= 0)
	{
		float ZDiff = Z - HeightVal;
		return SurvivalFPSMathFunctions::SmoothIntersection(DistDiff, ZDiff, 15) / 10.0f;
	}
	
	float ZDiff = - HeightVal - Z;
	return SurvivalFPSMathFunctions::SmoothIntersection(DistDiff, ZDiff, 15) / 10.0f;
}

FVoxelMaterial FVoxelGrasslandsIslandGeneratorInstance::GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD,
	const FVoxelItemStack& Items) const
{
	FVoxelMaterialBuilder Builder;

	// RGB
	//Builder.SetMaterialConfig(EVoxelMaterialConfig::RGB);
	//Builder.SetColor(FColor::Red);

	// Single index
	//Builder.SetMaterialConfig(EVoxelMaterialConfig::SingleIndex);
	//Builder.SetSingleIndex(0); 

	// Multi index
	Builder.SetMaterialConfig(EVoxelMaterialConfig::MultiIndex);
	Builder.AddMultiIndex(MaterialIndex, 1);

	return Builder.Build();
}

TVoxelRange<v_flt> FVoxelGrasslandsIslandGeneratorInstance::GetValueRangeImpl(const FVoxelIntBox& Bounds, int32 LOD,
	const FVoxelItemStack& Items) const
{
	// Return the values that GetValueImpl can return in Bounds
	// Used to skip chunks where the value does not change
	// Be careful, if wrong your world will have holes!
	// By default return infinite range to be safe
	return TVoxelRange<v_flt>::Infinite();

	// Example for the GetValueImpl above

	// Noise is between -1 and 1
	/*const TVoxelRange<v_flt> Height = TVoxelRange<v_flt>(-1, 1) * NoiseHeight;

	// Z can go from min to max
	TVoxelRange<v_flt> Value = TVoxelRange<v_flt>(Bounds.Min.Z, Bounds.Max.Z) - Height;

	Value /= 5;

	return Value;*/
}

FVector FVoxelGrasslandsIslandGeneratorInstance::GetUpVector(v_flt X, v_flt Y, v_flt Z) const
{
	return FVector::UpVector;
}
