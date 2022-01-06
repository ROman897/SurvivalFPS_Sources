// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShipPartSnapCollider.generated.h"


UENUM(BlueprintType)
enum class EShipPartSnapObjectType : uint8
{
	NONE,
	FLOOR,
	FLOOR_45DEG_TRIANGLE,
	FLOOR_22DEG_TRIANGLE,
	FLOOR_HALF,
	WALL_FULL,
	STAIRS_ON_FLOOR,
	TILTED_WALL,
	TRIANGLE_WALL
};

USTRUCT(BlueprintType)
struct FSnapPoint
{
	GENERATED_BODY()

	// TODO(Roman): use this
	UPROPERTY(EditAnywhere)
	FVector SnapLocationOffset;

	UPROPERTY(EditAnywhere)
	FRotator SnapRotation;

	UPROPERTY(EditAnywhere)
	int SnapMeshVariation;
};

UCLASS()
class SURVIVALFPS_API AShipPartSnapCollider : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* BottomCenter;

	UPROPERTY(VisibleAnywhere)
	class USceneComponent* FocusPoint;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* SnapCollider;

	UPROPERTY(EditDefaultsOnly)
	EShipPartSnapObjectType ShipPartSnapObjectType;

	UPROPERTY(EditDefaultsOnly)
	int ShipPartVariation;

public:
	// Sets default values for this actor's properties
	AShipPartSnapCollider();

	EShipPartSnapObjectType GetShipPartSnapObjectType() {
		return ShipPartSnapObjectType;
	}

	int GetShipPartVariation()
	{
		return ShipPartVariation;
	}

	FVector GetFocusPoint()
	{
		return FocusPoint->GetComponentLocation();
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
