// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShipData.generated.h"

UENUM(BlueprintType)
enum class EShipPartTypes : uint8
{
	ACTOR,
	FOUNDATION
};

UENUM(BlueprintType)
enum class EShipPartSnapTypes : uint8
{
	FLOOR,
	TRIANGLE_FLOOR,
	WALL,
	TILTED_WALL,
	WALL_TRIANGLE,
	ENUM_COUNT
};

static constexpr int SHIP_PART_SNAP_TYPES_COUNT = static_cast<int>(EShipPartSnapTypes::ENUM_COUNT);

USTRUCT(BlueprintType)
struct FSerializedShipFoundationPartData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FIntVector ShipTileCoords;

	UPROPERTY(EditAnywhere)
	int TileSnapId;
	
	UPROPERTY(EditAnywhere)
	class UShipPartDataAsset* ShipPartDataAsset;
};

USTRUCT(BlueprintType)
struct FShipPartSnapHandle
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	int ShipPartSnapType;
	
	UPROPERTY(EditAnywhere)
	FIntVector TileCoords;
	
	UPROPERTY(EditAnywhere)
	int SnapId;
};

struct FShipPartSnapHitResult
{
	bool bValid;
	FVector RelativeSnapLocation;
	FRotator RelativeSnapRotation;
	FVector RelativeSnapScale;

	FShipPartSnapHandle ShipPartSnapHandle;
};

