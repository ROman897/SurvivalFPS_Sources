// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FastNoise/VoxelFastNoise.h"
#include "VoxelGenerators/VoxelGeneratorHelpers.h"
#include "VoxelWorldGenerator.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API UVoxelWorldGenerator : public UVoxelGenerator
{
GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
	float NoiseHeight = 10.f;
	
	//~ Begin UVoxelGenerator Interface
	virtual TVoxelSharedRef<FVoxelGeneratorInstance> GetInstance() override;
	//~ End UVoxelGenerator Interface
};
    
class FMyVoxelWorldGeneratorInstance : public TVoxelGeneratorInstanceHelper<FMyVoxelWorldGeneratorInstance, UVoxelWorldGenerator>
{
public:
	using Super = TVoxelGeneratorInstanceHelper<FMyVoxelWorldGeneratorInstance, UVoxelWorldGenerator>;

	explicit FMyVoxelWorldGeneratorInstance(const UVoxelWorldGenerator& MyGenerator);

	//~ Begin FVoxelGeneratorInstance Interface
	virtual void Init(const FVoxelGeneratorInit& InitStruct) override;

	v_flt GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const;
	FVoxelMaterial GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const;

	TVoxelRange<v_flt> GetValueRangeImpl(const FVoxelIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const;

	virtual FVector GetUpVector(v_flt X, v_flt Y, v_flt Z) const override final;
	//~ End FVoxelGeneratorInstance Interface

private:
	const float NoiseHeight;
	const int32 Seed;
	FVoxelFastNoise Noise;
};
