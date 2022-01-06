// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
/*#include "BuildingPreviewBase.h"

#include "BuildingPreviewSplineBase.generated.h"

UCLASS()
class SURVIVALFPS_API ABuildingPreviewSplineBase : public ABuildingPreviewBase
{
	GENERATED_BODY()

// path consisting of circle - line - circle
struct FCSCPath
{
	FVector2D StartPoint;
	FVector2D StartDirection;
	FVector2D StartCircleCenter;
	FVector2D StartCircleTangentPoint;
	bool IsStartCircleRight;

	FVector2D EndPoint;
	FVector2D EndDirection;
	FVector2D EndCircleCenter;
	FVector2D EndCircleTangentPoint;
	bool IsEndCircleRight;

	float CircleRadius;
};


// path consisting of circle - circle - circle
struct FCCCPath
{
	FVector2D StartPoint;
	FVector2D StartDirection;
	FVector2D StartCircleCenter;
	FVector2D StartCircleTouchPoint;
	bool IsStartCircleRight;

	FVector2D MiddleCircleCenter;
	bool IsMiddleCircleRight;

	FVector2D EndPoint;
	FVector2D EndDirection;
	FVector2D EndCircleCenter;
	FVector2D EndCircleTouchPoint;
	bool IsEndCircleRight;

	float CircleRadius;
};

struct DesiredPlacementConfig
{
	//AConnectionPoint* HitConnectionPoint;
	FVector Location;
	FRotator Rotation;
	bool Valid = false;
};
	
protected:
	UPROPERTY(VisibleAnywhere)
	class USplineCreatorComponent* ConveyorSplineCreatorComponent;

	UPROPERTY(EditAnywhere)
	float DistToRecalculateSplineSqr;

	UPROPERTY(EditAnywhere)
	float SplineInitialLength;
	
	UPROPERTY(EditAnywhere)
	FVector SplineInitialDirection;

	UPROPERTY(EditAnywhere)
	float DefaultElevation;

	UPROPERTY(EditAnywhere)
	float MinDistBuildThresholdSqr;

	class AConnectionPoint* StartConnectionPoint;
	class AConnectionPoint* EndConnectionPoint;

	UPROPERTY(EditAnywhere)
	float ExtendConnectorStraightDistance;
	
	UPROPERTY(EditAnywhere)
	TArray<class UMaterialInterface*> ValidMaterials;

	UPROPERTY(EditAnywhere)
	TArray<class UMaterialInterface*> InvalidMaterials;

	UPROPERTY(EditAnywhere)
	float MaxTurnRadius;

	FVector EndDirection;

	bool bStartPlaced = false;

	FVector StartLocation;
	FVector StartDirection;

	UPROPERTY(EditAnywhere)
	float DesiredSplinePathDistBetweenPoints;

	TArray<FVector> CurrentSplinePath;

	UPROPERTY(EditAnywhere)
	float EndpointTangentSize;

	UPROPERTY(EditAnywhere)
	EConnectionPointType RequiredConnectionPointType;

	bool bIsColliding = false;

	DesiredPlacementConfig LastDesiredPlacementConfig;

	TSet<TPair<UPrimitiveComponent*, UPrimitiveComponent*>> OverlappedComponents;

public:
	ABuildingPreviewSplineBase();

	void SetDesiredPlacement(FVector CameraLocation, FVector CameraForward, int32 ZAxisRotationPresses) override;
	void LeftMouseClicked(APlayerCharacter* PlayerCharacter) override;
	bool AreConnectorDirectionsValid();

protected:
	void Reset();

	float GetAngleOnCircle(FVector2D CircleCenter, FVector2D PointOnCircle);
	
	void CalculateCSCTangents(const FCSCPath& CSCPath, TArray<FCSCPath>& ViableCSCPaths);

	void CalculateCCCTouchPoints(const FCCCPath& CCCPath, TArray<FCCCPath>& ViableCCCPaths);
	
	TArray<FVector2D> CalculateSplinePathFromCSCPath(const FCSCPath& CSCPath);
	TArray<FVector2D> CalculateSplinePathFromCCCPath(const FCCCPath& CCCPath);
	
	bool IsOnRightSideOfLine(FVector2D LineNormal, FVector2D LinePoint, FVector2D TestPoint);
	
	float AddPointsOnCircle(FVector2D CircleCenter, float CircleRadius, FVector2D StartPoint,
		FVector2D EndPoint, bool IsCircleRight, float SplinePathDistBetweenPoints, float InitDistToFirstPoint, bool IsLastInPath, TArray<FVector2D>& OutPathPoints);

	FVector2D GetPointOnCircle(FVector2D CircleCenter, float CircleRadius, float Angle);

	float GetCircleSegmentLength(FVector2D CircleCenter, FVector2D StartPoint, FVector2D EndPoint, bool IsCircleRight, float CircleRadius);

	float CalculateCSCPathLength(const FCSCPath& CSCPath);
	float CalculateCCCPathLength(const FCCCPath& CCCPath);
	float GetCirclePointsAngleDifference(float StartAngle, float EndAngle, bool IsCircleRight);

	float CalculateBestDistBetweenPathPoints(float PathLength);

	FCCCPath CalculateMidCircle(const FCCCPath& CCCPath, int Direction);

	bool DoCirclesIntersect(FVector2D Circle1Center, FVector2D Circle2Center, float CircleRadius);

	UFUNCTION()
	void OnSplineMeshBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSplineMeshEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
*/

