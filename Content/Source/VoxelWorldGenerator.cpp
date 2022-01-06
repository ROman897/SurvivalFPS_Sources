// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelWorldGenerator.h"
#include "FastNoise/VoxelFastNoise.inl"
#include "VoxelMaterialBuilder.h"
#include "WorldManager.h"

TVoxelSharedRef<FVoxelGeneratorInstance> UVoxelWorldGenerator::GetInstance()
{
	return MakeVoxelShared<FMyVoxelWorldGeneratorInstance>(*this);
}

///////////////////////////////////////////////////////////////////////////////

FMyVoxelWorldGeneratorInstance::FMyVoxelWorldGeneratorInstance(const UVoxelWorldGenerator& MyGenerator)
	: Super(&MyGenerator)
	, NoiseHeight(MyGenerator.NoiseHeight)
	, Seed(AWorldManager::GetInstance()->GetRandomStream().RandRange(TNumericLimits<int32>::Min(), TNumericLimits<int32>::Max()))
{
}

void FMyVoxelWorldGeneratorInstance::Init(const FVoxelGeneratorInit& InitStruct)
{
	check(AWorldManager::GetInstance() != nullptr);
	int32 NoiseSeed = AWorldManager::GetInstance()->GetRandomStream().RandRange(0, TNumericLimits<int32>::Max() - 1);
	Noise.SetSeed(NoiseSeed);
}

v_flt FMyVoxelWorldGeneratorInstance::GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
{
	//const float Height = Noise.GetPerlin_2D(X, Y, 0.01f);
	//const float Height = Noise.GetPerlinFractal_2D(X, Y, 0.01f, 3);
	
	check(AWorldManager::GetInstance() != nullptr);
	const FVoxelPresentBiomesData& VoxelBiomesData = AWorldManager::GetInstance()->GetBiomesAtVoxelPoint(FVector2D(X, Y));
	check(VoxelBiomesData.Biomes.Num() > 0);

	float FinalHeight = 0.0f;
	
	for (const FVoxelPresentBiomeData& VoxelBiomeData : VoxelBiomesData.Biomes)
	{
		float Height = 0.0f;
		for (const FBiomeHeightNoise& BiomeHeightNoise : VoxelBiomeData.BiomeRef->BiomeHeightNoises)
		{
			float LocalHeight;
			switch (BiomeHeightNoise.BiomeNoiseType)
			{
				case EBiomeNoiseType::PERLIN:
					LocalHeight = Noise.GetPerlin_2D(X, Y, BiomeHeightNoise.Frequency);
					break;
				case EBiomeNoiseType::PERLINFRACTAL:
					LocalHeight = Noise.GetPerlinFractal_2D(X, Y, BiomeHeightNoise.Frequency, BiomeHeightNoise.Octaves);
					break;
				case EBiomeNoiseType::IQ:
					LocalHeight = Noise.IQNoise_2D(X, Y, BiomeHeightNoise.Frequency, BiomeHeightNoise.Octaves);
					break;
				case EBiomeNoiseType::CUBIC_FRACTAL:
					LocalHeight = Noise.GetPerlinFractal_2D(X, Y, BiomeHeightNoise.Frequency, BiomeHeightNoise.Octaves);
					break;
				default:
					check(false);
			}
			Height += BiomeHeightNoise.TerrainHeightCurve->GetFloatValue(LocalHeight);
			
		}
		FinalHeight += VoxelBiomeData.BiomePercentageByStrength * Height;
	}

	// Positive value -> empty voxel
	// Negative value -> full voxel
	// Value positive when Z > Height, and negative Z < Height
	float Value = Z - FinalHeight;
	
	// The voxel value is clamped between -1 and 1. That can result in a bad gradient/normal. To solve that we divide it
	Value /= 5;

	return Value;
}

FVoxelMaterial FMyVoxelWorldGeneratorInstance::GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
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

	/*if (Z > NoiseHeight * 0.30f)
	{
		if (Z <= NoiseHeight * 0.37f)
		{
			float LowerIntensity = NoiseHeight * 0.37f - Z;
			float HigherIntensity = Z - NoiseHeight * 0.30f;
			Builder.AddMultiIndex(0, LowerIntensity);
			Builder.AddMultiIndex(1, HigherIntensity);
			
		} else
		{
			Builder.AddMultiIndex(1, 1.0f);
		}
	} else
	{
		Builder.AddMultiIndex(0, 1.0f);
	}
	return Builder.Build();*/

	check(AWorldManager::GetInstance() != nullptr);
	const FVoxelPresentBiomesData& VoxelBiomesData = AWorldManager::GetInstance()->GetBiomesAtVoxelPoint(FVector2D(X, Y));
	check(VoxelBiomesData.Biomes.Num() > 0);

	/*for (const FCloseBiome& BiomeProximity : CloseBiomes)
	{
		if (BiomeProximity.BiomePercentageByStrength > 0.518f)
		{
			Builder.AddMultiIndex(BiomeProximity.BiomeInstance->BiomeRef->MaterialIndex, 1.0f);
			return Builder.Build();
		}
	}*/

	if (VoxelBiomesData.Biomes.Num() >= 2)
	{
		Builder.AddMultiIndex(VoxelBiomesData.Biomes[0].MaterialId, VoxelBiomesData.Biomes[0].BiomePercentageByStrength);
		Builder.AddMultiIndex(VoxelBiomesData.Biomes[1].MaterialId, VoxelBiomesData.Biomes[1].BiomePercentageByStrength);
		//Builder.AddMultiIndex(3, 1.0f);
	} else
	{
		//Builder.AddMultiIndex(VoxelBiomesData.Biomes[0].BiomeRef->MaterialIndex, 1.0f);
	}

	return Builder.Build();
}

TVoxelRange<v_flt> FMyVoxelWorldGeneratorInstance::GetValueRangeImpl(const FVoxelIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const
{
	// Return the values that GetValueImpl can return in Bounds
	// Used to skip chunks where the value does not change
	// Be careful, if wrong your world will have holes!
	// By default return infinite range to be safe
	return TVoxelRange<v_flt>::Infinite();

	// Example for the GetValueImpl above

	// Noise is between -1 and 1
	const TVoxelRange<v_flt> Height = TVoxelRange<v_flt>(-1, 1) * NoiseHeight;

	// Z can go from min to max
	TVoxelRange<v_flt> Value = TVoxelRange<v_flt>(Bounds.Min.Z, Bounds.Max.Z) - Height;

	Value /= 5;

	return Value;
}

FVector FMyVoxelWorldGeneratorInstance::GetUpVector(v_flt X, v_flt Y, v_flt Z) const
{
	// Used by spawners
	return FVector::UpVector;
}