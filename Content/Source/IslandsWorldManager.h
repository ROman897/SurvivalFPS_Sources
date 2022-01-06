// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "VoxelWorld.h"
#include "GameFramework/Actor.h"
#include "Voronoi/Voronoi.h"

#include "IslandsWorldManager.generated.h"


struct FIslandBiomeInstance
{
	class UBiomeDataAsset* BiomeRef;

	FVector2D WorldLocation;

	int32 BiomeInstanceId;
};

UCLASS()
class SURVIVALFPS_API AIslandsWorldManager : public AActor
{
	GENERATED_BODY()

	struct FVoxelIsland
	{
		AVoxelWorld* VoxelWorld;
		float Radius;
	};

	struct FEnvironmentPartition
	{
		TArray<class AVoxelIslandWorld*> PresentVoxelWorlds;
		FVector2D BottomLeftLocation;
	};

	struct FIslandEnvironmentPartition
	{
		class AVoxelIslandWorld* PresentVoxelWorld;
		FVector2D BottomLeftLocation;
	};

protected:
	static AIslandsWorldManager* Instance;
	
	UPROPERTY(EditDefaultsOnly)
	float FillableWorldSize;
	
	// Curve must be defined at -0.5 to 0.5
	UPROPERTY(EditAnywhere)
	class UCurveFloat* TemperatureByYCoordCurve;
	
	UPROPERTY(EditAnywhere)
	TArray<class UBiomeDataAsset*> AllBiomeDataAssets;

	UPROPERTY(EditDefaultsOnly)
	int NumberOfBiomeInstances;
	
	TArray<FIslandBiomeInstance> BiomeInstances;

	UPROPERTY(Replicated)
	int32 WorldSeed;
	
	FRandomStream WorldRandomStream;

	UPROPERTY(EditDefaultsOnly)
	int NumberOfIslands;

	UPROPERTY(EditDefaultsOnly)
	float EnvironmentPartitionCellSize;

	TMap<TPair<int32, int32>, FEnvironmentPartition> EnvironmentPartitionCells;

	UPROPERTY(EditDefaultsOnly)
	float MinDistToLoadEnvironmentProp;

	int32 NumOfEnvironmentPartitionsToLoadAroundPlayer;

	TSet<TPair<int32, int32>> LoadedEnvironmentPartitions;

	TArray<FIslandEnvironmentPartition> IslandEnvironmentPartitionsToLoad;

	FTimerHandle SpawnIslandsEnvironmentsHandle;
	FTimerHandle LoadIslandEnvironmentPartitionTimerHandle;

	float MaxIslandRadiusInCm;

	TMap<TPair<int32, int32>, TArray<class AVoxelIslandWorld*>> VoxelIslandsPartitionCells;

public:
	// Sets default values for this actor's properties
	AIslandsWorldManager();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	static AIslandsWorldManager* GetInstance();
	FRandomStream& GetWorldRandomStream();

	TPair<int32, int32> GetEnvironmentPartitionCoords(FVector2D WorldCoords)
	{
		return TPair<int32, int32>(FMath::FloorToInt(WorldCoords.X / EnvironmentPartitionCellSize), FMath::FloorToInt(WorldCoords.Y / EnvironmentPartitionCellSize));
	}
	
	TPair<int32, int32> GetIslandPartitionCoords(FVector2D WorldCoords)
	{
		return TPair<int32, int32>(FMath::FloorToInt(WorldCoords.X / MaxIslandRadiusInCm), FMath::FloorToInt(WorldCoords.Y / MaxIslandRadiusInCm));
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void GenerateWorldFromSeed();
	void GenerateWorldOnServer();

	void GenerateBiomeInstancesOrigins();
	void SpawnIslands();

	void SpawnIslandsEnvironments();
	void LoadEnvironmentPartition(TPair<int32, int32> PartitionCoords);
	void LoadQueuedIslandEnvironmentPartitions();
	void SpawnIslandPartitionEnvironment(FIslandEnvironmentPartition& IslandEnvironmentPartition);
	FHitResult LineTraceGround(FVector2D Location);

	void AddHierarchicalInstancedMeshComponents();
};

