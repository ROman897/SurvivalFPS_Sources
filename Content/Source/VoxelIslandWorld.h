// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "VoxelWorld.h"
#include "GameFramework/Actor.h"
#include "VoxelIslandWorld.generated.h"

UCLASS()
class SURVIVALFPS_API AVoxelIslandWorld : public AVoxelWorld
{
	GENERATED_BODY()

public:
	int32 IslandRadiusInVoxels;
	class UBiomeDataAsset* IslandBiomeDataAsset;

public:
	// Sets default values for this actor's properties
	AVoxelIslandWorld();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
