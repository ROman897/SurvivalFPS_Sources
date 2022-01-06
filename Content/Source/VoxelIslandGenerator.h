// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * 
 */

#include "CoreMinimal.h"
#include "FastNoise/VoxelFastNoise.h"
#include "VoxelGenerators/VoxelGeneratorHelpers.h"
#include "VoxelIslandGenerator.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API UVoxelIslandGenerator : public UVoxelGenerator
{
GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	float IslandMinThickness;
};