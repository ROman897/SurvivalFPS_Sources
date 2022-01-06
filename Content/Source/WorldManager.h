// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BiomeDataAsset.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

#include <VoxelWorld.h>
#include "FastNoise/VoxelFastNoise.h"
#include <Kismet/KismetSystemLibrary.h>

#include "WorldManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWorldReady);

struct FBiomeInstance
{
	class UBiomeDataAsset* BiomeRef;

	FVector2D GlobalLocation;

	FVoxelFastNoise Noise;

	int32 BiomeInstanceId;
};

struct FVoxelPossibleBiomeData
{
	float Distance;

	float Strength;

	// in case multiple biomes overlap with given point,
	//get their normalized strength i.e. percentage by strength
	float BiomePercentageByStrength;
	
	const FBiomeInstance* BiomeInstance;
};

struct FVoxelPresentBiomeData
{
	// in case multiple biomes overlap with given point,
	//get their normalized strength i.e. percentage by strength
	float BiomePercentageByStrength;

	UBiomeDataAsset* BiomeRef;

	int MaterialId;
};

struct FVoxelBiomeTerrainData
{
	float Percentage;
	int MaterialId;
};

UENUM()
enum class ETerrainModificationType
{
	FLATTEN,
	DIG
};

USTRUCT()
struct FTerrainModification
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FVector Location;

	UPROPERTY(EditAnywhere)
	FVector Location2;

	UPROPERTY(EditAnywhere)
	float Strength;

	UPROPERTY(EditAnywhere)
	float Radius;

	UPROPERTY(EditAnywhere)
	float Falloff;

	UPROPERTY(EditAnywhere)
	EVoxelFalloff VoxelFalloff;

	ETerrainModificationType TerrainModificationType;
};

USTRUCT()
struct FSyncedTerrainModifications
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<FTerrainModification> TerrainModifications;

	UPROPERTY()
	uint32 Counter = 0;

	uint32 LastSyncedCounter;
};

static constexpr int32 MAX_NUMBER_OF_MIXING_BIOMES = 3;
	
struct FVoxelPresentBiomesData
{
	TArray<FVoxelPresentBiomeData, TInlineAllocator<MAX_NUMBER_OF_MIXING_BIOMES>> Biomes;
};

UCLASS()
class SURVIVALFPS_API AWorldManager : public AActor
{
	GENERATED_BODY()
	
	struct FBiomeRelativeEnvironmentPropsFrequency
	{
		float ScaledEnvironmentPropsFrequencyPer100m2;
		float BiomePercentageByPropsFrequency;
		const struct FBiomeInstance* BiomeInstance;
	};

	struct FEnvironmentPropScaledFrequency
	{
		float ScaledEnvironmentPropFrequencyPer100m2;
		const class UEnvironmentPropDataAsset* EnvironmentPropDataAsset; 
	};

	struct FBiomeApproximationSquare
	{
		TArray<const FBiomeInstance*> PossibleBiomes;
	};

	struct FSerializedEnemyCamp
	{
		FVector Location;
		FEnemyCampData EnemyCampData;
	};

	struct FWorldCompositionTile
	{
		TArray<FSerializedEnemyCamp> SerializedEnemyCamps;
		TArray<class AEnemyCamp*> SpawnedEnemyCamps;
	};

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AVoxelWorld> VoxelWorldClass;

	UPROPERTY(BlueprintReadOnly)
	AVoxelWorld* VoxelWorld;

	UPROPERTY(ReplicatedUsing = OnRepSyncedTerrainModifications)
	FSyncedTerrainModifications SyncedTerrainModifications;
		
	UPROPERTY(EditAnywhere)
	TArray<class UBiomeDataAsset*> AllBiomes;
	
	TArray<FBiomeInstance> BiomeInstances;

	UPROPERTY(EditAnywhere)
	int AtomicEnvironmentSubregionSizeInVoxels;

	static AWorldManager* Instance;

	bool EnvironmentGenerated = false;

	FTimerHandle SpawnEnvironmentDelayHandle;
	FTimerHandle SpawnActorsDelayHandle;

	UPROPERTY(EditAnywhere)
	float MaxBiomeOverlapInWorldUnits;

	UPROPERTY(EditAnywhere)
	int BiomeDistanceExponent;

	// Curve must be defined at -0.5 to 0.5
	UPROPERTY(EditAnywhere)
	class UCurveFloat* TemperatureByYCoordCurve;

	UPROPERTY(EditAnywhere)
	int NumberOfBiomeInstances;

	UPROPERTY(EditAnywhere)
	float BiomeDistanceNoise;

	UPROPERTY(EditAnywhere)
	float BiomeDistanceNoiseFrequency;

	TArray<FVoxelPresentBiomesData> VoxelsBiomesData;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AEnemyCamp> EnemyCampClass;

	UPROPERTY(EditAnywhere)
	float EnemyCampMinDistanceFromPlayerSpawn;

	float EnemyCampMinDistanceFromPlayerSpawnSqr;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> BiomeOriginDebugActor;

	// must be multiple of WorldSizeInVoxels
	UPROPERTY(EditAnywhere)
	int32 BiomeApproximationSquareSize;

	int32 BiomeApproximationSquaresPerSide;

	TArray<FBiomeApproximationSquare> BiomeApproximationSquares;
	
	UPROPERTY(EditAnywhere)
	float WorldCompositionTileSize;

	// if a player is on a given tile, how many tiles around it to load
	// loads all tiles that share point/side with the given tile
	UPROPERTY(EditAnywhere)
	int WorldCompositionNumTilesToLoadAroundPlayer;

	TArray<AActor*> RegisteredPlayers;

	TMap<FIntPoint, FWorldCompositionTile> WorldCompositionTiles;
	
	TSet<FIntPoint> WorldCompositionLoadedTiles;

	UPROPERTY(EditAnywhere)
	float RecalculateWorldCompositionTilesTime;

	FTimerHandle RecalculateWorldCompositionTilesHandle;

	UPROPERTY(EditAnywhere)
	float ClearDistanceFromPlayerSpawn;

	float ClearDistanceFromPlayerSpawnSqr;

	UPROPERTY(Replicated)
	int32 WorldSeed;

	FRandomStream WorldRandomStream;

	bool bReady = false;

	TArray<class UItemDataAsset*> AllItemDataAssets;

public:
	FWorldReady WorldReadyDelegate;

public:
	// Sets default values for this actor's properties
	AWorldManager();
	virtual void Tick(float DeltaTime) override;

	AVoxelWorld* GetVoxelWorld();

	void GenerateNewWorld();

	void GenerateWorldFromSeed();

	void AddTerrainModification(const FTerrainModification& TerrainModification);

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents() override;

	bool IsReady();

	UFUNCTION()
	void RecalculateWorldCompositionTiles();

	static AWorldManager* GetInstance()
	{
		return Instance;
	}

	const FVoxelPresentBiomesData& GetBiomesAtVoxelPoint(FVector2D VoxelPoint);
	
	bool GetSurfaceLocation(FVector& OutLocation);

	void RecalculateNavigationUnderActor(AActor* TargetActor);
	void RecalculateNavigationInVolumes(const TArray<FBox>& Volumes);
	void RecalculateNavigationInVoxelBox(const FVoxelIntBox VoxelIntBox);
	void RecalculateNavigationInVoxelBoxes(const TArray<FVoxelIntBox>& VoxelIntBoxes);
	
	void DebugCalculateBiomesAtLocation(FVector Location);

	void RegisterPlayer(AActor* Player);

	FHitResult LineTraceGround(FVector2D Location);

	FRandomStream& GetRandomStream();

	const TArray<class UItemDataAsset*>& GetAllItemDataAssets();

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRepSyncedTerrainModifications();

	void Flatten(const FTerrainModification& TerrainModification);
	
	void Dig(const FTerrainModification& TerrainModification);

	int32 GetVoxelIndex2D(FIntVector VoxelCoords);
	
	FIntVector GetApproxSquareStartCoords(int ApproxSquareId);

	UFUNCTION()
	void TerrainGenerated();

	void GenerateNewEnvironment();

	// since we have a large number of voxels, first calculate biomes that can be possible in range
	// for voxels in a given square
	void CalculateBiomeApproximationSquares();
	
	void CalculateBiomesAtVoxels();
	void CalculateBiomesAtApproxSquare(int ApproxSquareId);
	void CalculateBiomesAtYCoord(int32 YCoord);

	UFUNCTION()
	void SpawnActors();

	void GenerateNavigation();
	void RebuildNavigation();

	void GenerateEnemyCamps();

	void GenerateBiomeInstancesOrigins();
	
	void AddHierarchicalInstancedMeshComponents();

	void LoadTile(FIntPoint TileCoord);
	void UnloadTile(FIntPoint TileCoord);
	FIntPoint WorldToTile(FVector Location);

	UFUNCTION()
	void NavOctreeBuildComplete(bool Successful);

};

