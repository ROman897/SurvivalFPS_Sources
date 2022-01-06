// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "SurvivalFPS/VoxelIslandGenerator.h"
#include "FastNoise/VoxelFastNoise.h"
#include "VoxelGenerators/VoxelGeneratorHelpers.h"
#include "VoxelDesertIslandGenerator.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API UVoxelDesertIslandGenerator : public UVoxelIslandGenerator
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	float TopNoiseHeight = 5.f;
	
	UPROPERTY(EditAnywhere)
	float TopNoiseFrequency;

	UPROPERTY(EditAnywhere)
	int TopNoiseOctaves;
	
	UPROPERTY(EditAnywhere)
	float BottomNoiseHeight = 5.f;
	
	UPROPERTY(EditAnywhere)
	float BottomNoiseFrequency;

	UPROPERTY(EditAnywhere)
	int BottomNoiseOctaves;

	UPROPERTY(EditAnywhere)
	float BordersLargeFrequency;
	
	UPROPERTY(EditAnywhere)
	float BordersLargeOctaves;
	
	UPROPERTY(EditAnywhere)
	float BordersLargePercentageOfRadius;
	
	UPROPERTY(EditAnywhere)
	float BordersSmallFrequency;

	UPROPERTY(EditAnywhere)
	float BordersSmallOctaves;

	UPROPERTY(EditAnywhere)
	float BordersSmallNoiseHeight;
	
	UPROPERTY(EditAnywhere)
	float VerticalNoiseFrequency;
	
	UPROPERTY(EditAnywhere)
	float VerticalNoiseOctaves;
	
	UPROPERTY(EditAnywhere)
	float VerticalNoiseHeight;

	UPROPERTY(EditAnywhere)
	int MaterialIndex;

public:
	virtual TVoxelSharedRef<FVoxelGeneratorInstance> GetInstance() override;
};

class FVoxelDesertIslandGeneratorInstance : public TVoxelGeneratorInstanceHelper<FVoxelDesertIslandGeneratorInstance, UVoxelDesertIslandGenerator>
{
public:
	using Super = TVoxelGeneratorInstanceHelper<FVoxelDesertIslandGeneratorInstance, UVoxelDesertIslandGenerator>;

public:
	const float TopNoiseHeight;
	
	FVoxelFastNoise TopNoise;
	FVoxelFastNoise BottomNoise;
	const float TopNoiseFrequency;
	const int TopNoiseOctaves;

	const float BottomNoiseHeight;
	const float BottomNoiseFrequency;
	const int BottomNoiseOctaves;

	const float BordersLargeFrequency;
	const float BordersLargeOctaves;
	const float BordersLargePercentageOfRadius;
	FVoxelFastNoise BordersLargeNoise;

	const float BordersSmallFrequency;
	const float BordersSmallOctaves;
	const float BordersSmallNoiseHeight;
	FVoxelFastNoise BordersSmallNoise;
	
	const float VerticalNoiseFrequency;
	const float VerticalNoiseOctaves;
	const float VerticalNoiseHeight;
	FVoxelFastNoise VerticalNoise;
	
	int MaterialIndex;
	int IslandRadiusInVoxels;
	const float IslandMinThickness;

public:
	explicit FVoxelDesertIslandGeneratorInstance(const UVoxelDesertIslandGenerator& MyGenerator);
	virtual void Init(const FVoxelGeneratorInit& InitStruct) override;
	
	v_flt GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const;
	FVoxelMaterial GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const;
	TVoxelRange<v_flt> GetValueRangeImpl(const FVoxelIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const;
	virtual FVector GetUpVector(v_flt X, v_flt Y, v_flt Z) const override final;
};
