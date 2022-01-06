// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "VoxelMeshData.h"
#include "VoxelMeshDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API UVoxelMeshDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	FVoxelMeshData VoxelMeshData;

	UPROPERTY(EditAnywhere)
	UStaticMesh* SourceStaticMesh;

	// stores voxel size we used when converting from source static mesh to voxel data
	UPROPERTY(EditAnywhere)
	float SourceVoxelSize;
};
