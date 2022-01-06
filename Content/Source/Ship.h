// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "DynamicMesh3.h"
#include "GameFramework/Actor.h"
#include "Items/Assets/ShipPartDataAsset.h"
#include "ShipData.h"
#include "Containers/Union.h"

#include "Ship.generated.h"

// set to 2, because there can be two floor triangles in one tile
static constexpr int MAX_NUM_OF_SAME_FOUNDATION_ON_ONE_TILE= 2;
static constexpr int MAX_NUM_OF_SAME_FOUNDATION_COLLIDERS_ON_ONE_TILE= 2;
static constexpr float TILE_LENGTH = 100.0f;


USTRUCT()
struct FShipFoundationPart
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	class UShipPartDataAsset* ShipPartDataAsset;
	
	UPROPERTY(EditAnywhere)
	int TileSnapId;

	UPROPERTY()
	class UStaticMeshComponent* ShipPartDestructibleMeshComponent;
};

USTRUCT()
struct FShipPartsOneSnapType
{
	GENERATED_BODY()

	// spawned ship parts
	TArray<FShipFoundationPart, TInlineAllocator<MAX_NUM_OF_SAME_FOUNDATION_ON_ONE_TILE>> ShipFoundationParts;

	// stores if particular snap is active/spawnable
	TArray<bool, TInlineAllocator<MAX_NUM_OF_SAME_FOUNDATION_ON_ONE_TILE>> SpawnableShipFoundationSnapColliders;

	// stores array of spawned snaps
	TArray<UStaticMeshComponent*, TInlineAllocator<MAX_NUM_OF_SAME_FOUNDATION_ON_ONE_TILE>> SpawnedShipFoundationSnapColliders;

	// true if ship part preview of this snap type is active and this tile is inside line trace bounds
	bool bCollidersSpawned = false;
};


UENUM(BlueprintType)
enum class EEdgeTypes : uint8
{
	FORWARD,
	RIGHT,
	DIAGONAL_F_R,
	DIAGONAL_F_L,
	WALL_FORWARD_DIAGONAL_D_R, // diagonal that is in wall of forward edge, going from down right point
	WALL_FORWARD_DIAGONAL_D_L, // diagonal that is in wall of forward edge, going from down left point
	WALL_RIGHT_DIAGONAL_D_F, // diagonal that is in wall of right edge, going from down forward point
	WALL_RIGHT_DIAGONAL_D_B, // diagonal that is in wall of right edge, going from down backward point
	WALL_CUBE_DIAGONAL_F_R,
	WALL_CUBE_DIAGONAL_F_L,
	WALL_CUBE_DIAGONAL_B_R,
	WALL_CUBE_DIAGONAL_B_L,
	ENUM_COUNT
};

static constexpr int EDGE_TYPES_COUNT = static_cast<int>(EEdgeTypes::ENUM_COUNT);

USTRUCT(BlueprintType)
struct FSnapColliderEdge
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	EEdgeTypes EdgeType;

	UPROPERTY(EditAnywhere)
	FIntVector TileOffset;

	UPROPERTY(EditAnywhere)
	bool bCanUseForSnap = true;
};

struct FTileLocalSnapId
{
	EShipPartSnapTypes SnapType;
	int SnapId;
};

USTRUCT()
struct FShipTile
{
	GENERATED_BODY()

	FShipPartsOneSnapType ShipParts[EShipPartSnapTypes::ENUM_COUNT];
	bool HasEdges[EEdgeTypes::ENUM_COUNT];
	bool bTileHasRoof = false;
};

USTRUCT(BlueprintType)
struct FShipPartSnapColliderDefinition
{
	GENERATED_BODY()

	int SnapType;
	int SnapId;

	UPROPERTY(EditAnywhere)
	UStaticMesh* ColliderMesh;
	
	UPROPERTY(EditAnywhere)
	FVector ColliderOffsetFromTile = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere)
	FRotator ColliderRelativeRotation = FRotator::ZeroRotator;
	
	UPROPERTY(EditAnywhere)
	FVector ColliderScale = FVector::OneVector;

	UPROPERTY(EditAnywhere)
	FVector SnapPointOffsetFromTile = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere)
	FRotator SnapPointRelativeRotation = FRotator::ZeroRotator;
	
	UPROPERTY(EditAnywhere)
	FVector SnapPointScale = FVector::OneVector;

	UPROPERTY(EditAnywhere)
	TArray<FSnapColliderEdge> Edges;

	// blocked snap colliders, TileCoords is set to relative from tile
	// that this snap collider is at
	UPROPERTY(EditAnywhere)
	TArray<FShipPartSnapHandle> BlockedSnapsRelativeTileCoords;
};
	
USTRUCT()
struct FSnapDefinitionsArray
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<FShipPartSnapColliderDefinition> ShipPartSnapColliderDefinitions;
};

struct FShipActorPartHandle
{
	AShipPart* ShipPartActor;
};

inline bool operator==(FShipActorPartHandle Lhs, FShipActorPartHandle Rhs)
{
	return Lhs.ShipPartActor == Rhs.ShipPartActor;
}

struct FShipFoundationPartHandle
{
	UStaticMeshComponent* ShipFoundationPartStaticMesh;
};

inline bool operator==(FShipFoundationPartHandle Lhs, FShipFoundationPartHandle Rhs)
{
	return Lhs.ShipFoundationPartStaticMesh == Rhs.ShipFoundationPartStaticMesh;
}

using FShipPartHandle = TUnion<FShipActorPartHandle, FShipFoundationPartHandle>;

UCLASS()
class SURVIVALFPS_API AShip : public APawn
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* BoundaryBoxComponent;

	UPROPERTY(VisibleAnywhere)
	TArray<UChildActorComponent*> ShipPartChildActorComponents;

	UPROPERTY()
	class AShipControls* ShipControls;

	UPROPERTY(VisibleAnywhere)
	class UShipFloatingMovement* ShipFloatingMovement;

	float RequiredPowerForFullThrottlePerSecond = 0.0f;

	TSet<class IPowerProducer*> PowerProducers;

	bool bIsPowered = false;

	TSet<class AShipPart*> ShipPartChildActors;

	TArray<class AShipEngine*> ShipEngines;

	TMap<FIntVector, FShipTile> ShipTiles;

	UPROPERTY(EditDefaultsOnly)
	int StartFloorHalfWidth;
	
	UPROPERTY(EditDefaultsOnly)
	int StartFloorHalfLength;

	UPROPERTY(EditDefaultsOnly)
	UShipPartDataAsset* StartFloorShipPartDataAsset;

	TMap<UPrimitiveComponent*, FShipPartSnapHandle> SnapColliderToSnapHandle;

	UPROPERTY(EditDefaultsOnly)
	TMap<EShipPartSnapTypes, FSnapDefinitionsArray> AllSnapDefinitionsMap;

	FSnapDefinitionsArray AllSnapDefinitionsArray[SHIP_PART_SNAP_TYPES_COUNT];

	bool bHasSpawnedSnapCollidersType;
	int SpawnedSnapCollidersType;
	FIntVector PlayerTileCoordsForSnap;
	float CurrentSnapTypeMaxRange;

	TMap<UStaticMeshComponent*, FShipPartSnapHandle> FoundationMeshToSnapHandle;

	FIntVector BottomLeftTileBound;
	FIntVector TopRightTileBound;

public:
	// Sets default values for this actor's properties
	AShip();
	virtual void Tick(float DeltaTime) override;

	class UShipFloatingMovement* GetShipFloatingMovement();
	
	void AddSteering(float Direction);
	void AddThrottle(float Direction);
	void AddPitch(float Direction);

	void AddShipActorPart(class UShipPartDataAsset* ShipPartDataAsset, FVector Location, FRotator Rotation);
	bool AddFoundationShipPart(const FSerializedShipFoundationPartData& ShipFoundationPartData);

	void AddPowerProducer(class IPowerProducer* PowerProducer);
	void RemovePowerProducer(class IPowerProducer* PowerProducer);
	
	float ConsumeEnginePower(float ThrottleAmount, float DeltaTime);

	static AShip* FindParentShip(AActor* Actor);

	FShipPartSnapHitResult EvaluateSnapHit(const FHitResult& HitResult);

	void ShipHitWithBullet(UPrimitiveComponent* HitComponent, const FTransform& DestructionCapsuleTransform, float CapsuleHeight, float CapsuleRadius);

	void EnableSnapsOfType(EShipPartSnapTypes ShipPartSnapType, FVector PlayerLocation, float MaxRange);
	void EnableSnapsOfType(int ShipPartSnapType, FIntVector CurrentPlayerTileCoords, float MaxRange);
	void DisableSnapsOfType();
	bool GetShipPartHandleFromHit(const FHitResult& HitResult, FShipPartHandle& ShipPartHandle);
	void DeconstructShipPart(FShipPartHandle ShipPartHandle);

	UPrimitiveComponent* GetMovingPrimitiveComponent();

	bool IsPointInside(FVector Location);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void CalculateEngineStats();

	void SpawnStartingShipFoundations();

	void SpawnShipFoundationPart(const FSerializedShipFoundationPartData& ShipFoundationPartData);

	FVector GetRelativeTileLocation(FIntVector TileCoords)
	{
		return FVector(TileCoords) * TILE_LENGTH;
	}

	FIntVector GetTileCoordsFromWorldLocation(FVector WorldLocation);

	void UpdateSnapCollidersAtTile(FIntVector TileCoords);

	void UpdateHasEdgesAtTile(FIntVector TileCoords);

	FShipTile& FindOrAddShipTile(FIntVector TileCoords);
	
	UStaticMeshComponent* AddSnapCollider(FShipPartSnapHandle ShipPartSnapHandle);
	void DestroySnapCollider(UStaticMeshComponent* ColliderMeshComponent);

	const FShipPartSnapColliderDefinition& FindShipPartSnapColliderDefinition(int ShipPartSnapType, int SnapId) const;

	bool IsSnapColliderConnectedByEdge(FIntVector TileCoords, const FShipPartSnapColliderDefinition& ShipPartSnapColliderDefinition);
	bool IsSnapColliderBlocked(FIntVector TileCoords, const FShipPartSnapColliderDefinition& ShipPartSnapColliderDefinition);

	void FillAllSnapDefinitionsArray();

	void ThrottleValueChanged(float ThrottleValue);

	// this means this snap type on this tile should be spawned
	void UpdateSnapColliderComponentsOfType(FIntVector TileCoords, int ShipPartSnapType);

	// called when snap type on this tile should be destroyed
	void DestroySnapColliderComponentsOfType(FIntVector TileCoords, int ShipPartSnapType);

	void TileFoundationsChanged(FIntVector TileCoords);

	bool IsIntVectorInBounds(FIntVector LowerBound, FIntVector UpperBoundInclusive, FIntVector Point)
	{
		return LowerBound.X <= Point.X
			&& LowerBound.Y <= Point.Y
			&& LowerBound.Z <= Point.Z
			&& UpperBoundInclusive.X >= Point.X
			&& UpperBoundInclusive.Y >= Point.Y
			&& UpperBoundInclusive.Z >= Point.Z;
	}

	void UpdateTileBounds();

	void UpdateRoofsAtTile(int XCoord, int YCoord);
};