// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "BiomeDataAsset.generated.h"

USTRUCT(BlueprintType)
struct SURVIVALFPS_API FEnvironmentPropSpawnAttributes
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float EnvironmentPropFrequencyPer100m2;
	
	UPROPERTY(EditAnywhere)
	class UEnvironmentPropDataAsset* EnvironmentPropDataAsset;
};

USTRUCT()
struct FEnemyCampData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float Probability;

	UPROPERTY(EditAnywhere)
	int32 EnemyCountLowerBound;

	UPROPERTY(EditAnywhere)
	int32 EnemyCountUpperBound;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APawn>> SpawnableEnemies;
};

UENUM(BlueprintType)
enum class EBiomeNoiseType : uint8
{
	PERLIN,
	PERLINFRACTAL,
	IQ,
	CUBIC_FRACTAL
};

USTRUCT(BlueprintType)
struct FBiomeHeightNoise
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EBiomeNoiseType BiomeNoiseType;
	
	UPROPERTY(EditAnywhere)
	class UCurveFloat* TerrainHeightCurve;

	UPROPERTY(EditAnywhere)
	float Frequency;
	
	UPROPERTY(EditAnywhere)
	float Octaves;
};

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API UBiomeDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<FEnvironmentPropSpawnAttributes> SpawnableEnvironmentProps;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AVoxelWorld> VoxelWorldClass;

	UPROPERTY(EditAnywhere)
	TArray<FBiomeHeightNoise> BiomeHeightNoises;

	UPROPERTY(EditAnywhere)
	class UCurveFloat* BiomeStrengthByTemperatureCurve;

	UPROPERTY(EditAnywhere)
	float EnemyCampsFrequencyPer100m2;

	UPROPERTY(EditAnywhere)
	TArray<FEnemyCampData> EnemyCamps;

	UPROPERTY(EditAnywhere)
	int32 MinIslandRadiusInVoxels;
	
	UPROPERTY(EditAnywhere)
	int32 MaxIslandRadiusInVoxels;
};
