// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelDesertIslandGenerator.h"

#include "VoxelMaterialBuilder.h"
#include "FastNoise/VoxelFastNoise.inl"
#include "SurvivalFPS/IslandsWorldManager.h"
#include "SurvivalFPS/SurvivalFPSMathFunctions.h"
#include "SurvivalFPS/VoxelIslandWorld.h"

TVoxelSharedRef<FVoxelGeneratorInstance> UVoxelDesertIslandGenerator::GetInstance()
{
	return MakeVoxelShared<FVoxelDesertIslandGeneratorInstance>(*this);
}

FVoxelDesertIslandGeneratorInstance::FVoxelDesertIslandGeneratorInstance(const UVoxelDesertIslandGenerator& MyGenerator)
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

void FVoxelDesertIslandGeneratorInstance::Init(const FVoxelGeneratorInit& InitStruct)
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

v_flt FVoxelDesertIslandGeneratorInstance::GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD,
                                                        const FVoxelItemStack& Items) const
{
	// NOTE(Roman): this is the basic implementation from floating island tutorial
	
	/*float BorderNoiseMain = BordersNoise.GetSimplexFractal_2D(X, Y, BordersFrequency, 3);
	//float BorderNoiseMain = BordersNoise.GetSimplex_2D(X, Y, BordersFrequency);
	
	float ActualDist = FVector2D(X, Y).Size();
	
	float SmoothBordersNoise1 = BordersNoise.GetPerlin_3D(X / 20, Y / 20, Z / 20, BordersFrequency * 4);
	float SmoothBordersNoise2 = BordersNoise.GetPerlin_3D(X / 5, Y / 5, Z / 5, BordersFrequency * 2);

	float BorderNoiseVal = (BorderNoiseMain - SmoothBordersNoise1 - SmoothBordersNoise2) * 0.75f;

	float LerpedValue = FMath::Lerp(-1.0f, BorderNoiseVal, (ActualDist / IslandRadiusInVoxels) / 0.69f);
	
	float PositiveLerpedNoise = FMath::Clamp(FMath::Abs(FMath::Min(LerpedValue, 0.0f)), 0.0f, 1.0f);

	if (ActualDist > IslandRadiusInVoxels || PositiveLerpedNoise < 0.1f)
	{
		return 1;
	}
	
	float Value;
	if (Z >= 0)
	{
		float FinalTopHeight = TopNoise.GetPerlinFractal_2D(X, Y, TopNoiseFrequency, TopNoiseOctaves) * NoiseHeight + IslandMinThickness;
		Value = Z - FinalTopHeight;
	} else
	{
		float FinalBottomHeight = BottomNoise.GetSimplexFractal_2D(X, Y, BottomNoiseFrequency, BottomNoiseOctaves) * NoiseHeight + IslandMinThickness;

		// here Z is negative, FinalBottomHeight is strictly positive
		// and we put voxel if value is negative
		Value = -Z - FinalBottomHeight;
	}

	// Positive value -> empty voxel
	// Negative value -> full voxel
	// Value positive when Z > Height, and negative Z < Height
	
	// The voxel value is clamped between -1 and 1. That can result in a bad gradient/normal. To solve that we divide it
	Value /= 5;

	return Value;
	*/

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
		//float HeightVal = TopNoise.GetSimplex_2D(X, Y, TopNoiseFrequency) * TopNoiseHeight + IslandMinThickness * 0.5f;
		float ZDiff = Z - HeightVal;
		return SurvivalFPSMathFunctions::SmoothIntersection(DistDiff, ZDiff, 15) / 10.0f;
	}
	
	//float HeightVal = BottomNoise.GetSimplex_2D(X, Y, BottomNoiseFrequency) * BottomNoiseHeight + IslandMinThickness * 0.5f;
	float ZDiff = - HeightVal - Z;
	return SurvivalFPSMathFunctions::SmoothIntersection(DistDiff, ZDiff, 15) / 10.0f;

}

FVoxelMaterial FVoxelDesertIslandGeneratorInstance::GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD,
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

TVoxelRange<v_flt> FVoxelDesertIslandGeneratorInstance::GetValueRangeImpl(const FVoxelIntBox& Bounds, int32 LOD,
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

FVector FVoxelDesertIslandGeneratorInstance::GetUpVector(v_flt X, v_flt Y, v_flt Z) const
{
	return FVector::UpVector;
}