// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildingPreviewSplineBase.h"
#include "SplineCreatorComponent.h"
#include "PlayerCharacter.h"
/*

#include <Containers/Array.h>

#include "IndexSplineMeshComponent.h"

#define TRACE_CHANNEL_BUILDING_CONNECTION ECC_GameTraceChannel8

ABuildingPreviewSplineBase::ABuildingPreviewSplineBase() {
	ConveyorSplineCreatorComponent = CreateDefaultSubobject<USplineCreatorComponent>(TEXT("ConveyorSplineCreatorComp"));
}

void ABuildingPreviewSplineBase::SetDesiredPlacement(FVector CameraLocation, FVector CameraForward, int32 ZAxisRotationPresses)
{
	FRotator Rotation(0, ZAxisRotationPresses, 0) ;
	FHitResult TargetHit = LineTraceMaxDistFromPlayer(CameraLocation, CameraForward, TRACE_CHANNEL_BUILDING_CONNECTION);
	const FVector ElevationVec = FVector(0, 0, DefaultElevation);
	const FVector ElevatedTargetLocation = TargetHit.Location + ElevationVec;

	if (TargetHit.Actor == nullptr)
	{
		return;
	}
	/*AConnectionPoint* HitConnectionPoint = Cast<AConnectionPoint>(TargetHit.Actor);

	if (HitConnectionPoint != nullptr && HitConnectionPoint->GetConnectionPointType() != RequiredConnectionPointType)
	{
		HitConnectionPoint = nullptr;
	}

	if (LastDesiredPlacementConfig.Valid
		&& TargetHit.Location == LastDesiredPlacementConfig.Location
		&& Rotation == LastDesiredPlacementConfig.Rotation
		&& LastDesiredPlacementConfig.HitConnectionPoint == HitConnectionPoint)
	{
		return;
	}

	LastDesiredPlacementConfig.Valid = true;
	LastDesiredPlacementConfig.HitConnectionPoint = HitConnectionPoint;
	LastDesiredPlacementConfig.Location = TargetHit.Location;
	LastDesiredPlacementConfig.Rotation = Rotation;

	if (!bStartPlaced)
	{
		TArray<FVector> SplinePoints;
		StartConnectionPoint = HitConnectionPoint;
		
		if (StartConnectionPoint != nullptr)
		{
			SplinePoints = {StartConnectionPoint->GetActorLocation(), StartConnectionPoint->GetActorLocation() + StartConnectionPoint->GetConnectingDirection() * SplineInitialLength};
			ConveyorSplineCreatorComponent->SetTangentAtSplinePoint(0, 1000 * StartConnectionPoint->GetConnectingDirection(), ESplineCoordinateSpace::World);
		} else
		{
			SplinePoints = {ElevatedTargetLocation, ElevatedTargetLocation + SplineInitialDirection * SplineInitialLength};
		}
		ConveyorSplineCreatorComponent->SetSplinePoints(SplinePoints, ESplineCoordinateSpace::World);
	} else
	{
		EndConnectionPoint = HitConnectionPoint;

		FVector TargetLocation;
		if (EndConnectionPoint == nullptr)
		{
			TargetLocation = ElevatedTargetLocation;
			FVector2D RotatedEndDirection2D = FVector2D(StartDirection).GetSafeNormal().GetRotated(Rotation.Yaw);
			EndDirection = FVector(RotatedEndDirection2D.X, RotatedEndDirection2D.Y, 0.0f);
		} else
		{
			TargetLocation = EndConnectionPoint->GetActorLocation();
			EndDirection = -EndConnectionPoint->GetConnectingDirection();
		}

		FVector2D StartLocation2D(StartLocation);
		FVector2D EndLocation2D(TargetLocation);
		
		FVector2D StartDirection2D = FVector2D(StartDirection).GetSafeNormal();
		FVector2D CircleCenterStartRight = StartLocation2D + FVector2D(StartDirection2D.Y, -StartDirection2D.X) * MaxTurnRadius;
		FVector2D CircleCenterStartLeft = StartLocation2D + FVector2D(-StartDirection2D.Y, StartDirection2D.X) * MaxTurnRadius;

		FVector2D EndDirection2D = FVector2D(EndDirection).GetSafeNormal();
		FVector2D CircleCenterEndRight = EndLocation2D + FVector2D(EndDirection2D.Y, -EndDirection2D.X) * MaxTurnRadius;
		FVector2D CircleCenterEndLeft = EndLocation2D + FVector2D(-EndDirection2D.Y, EndDirection2D.X) * MaxTurnRadius;

		// calculate CSC Paths
		FCSCPath CommonBasePath;
		CommonBasePath.StartPoint = StartLocation2D;
		CommonBasePath.StartDirection = StartDirection2D;
		CommonBasePath.EndPoint = EndLocation2D;
		CommonBasePath.EndDirection = EndDirection2D;
		CommonBasePath.CircleRadius = MaxTurnRadius;

		FCSCPath RSRPath = CommonBasePath;
		RSRPath.StartCircleCenter = CircleCenterStartRight;
		RSRPath.IsStartCircleRight = true;
		RSRPath.EndCircleCenter = CircleCenterEndRight;
		RSRPath.IsEndCircleRight = true;

		FCSCPath RSLPath = CommonBasePath;
		RSLPath.StartCircleCenter = CircleCenterStartRight;
		RSLPath.IsStartCircleRight = true;
		RSLPath.EndCircleCenter = CircleCenterEndLeft;
		RSLPath.IsEndCircleRight = false;

		FCSCPath LSRPath = CommonBasePath;
		LSRPath.StartCircleCenter = CircleCenterStartLeft;
		LSRPath.IsStartCircleRight = false;
		LSRPath.EndCircleCenter = CircleCenterEndRight;
		LSRPath.IsEndCircleRight = true;

		FCSCPath LSLPath = CommonBasePath;
		LSLPath.StartCircleCenter = CircleCenterStartLeft;
		LSLPath.IsStartCircleRight = false;
		LSLPath.EndCircleCenter = CircleCenterEndLeft;
		LSLPath.IsEndCircleRight = false;
		
		TArray<FCSCPath> ViableCSCPaths;

		CalculateCSCTangents(RSRPath, ViableCSCPaths);
		CalculateCSCTangents(RSLPath, ViableCSCPaths);
		CalculateCSCTangents(LSRPath, ViableCSCPaths);
		CalculateCSCTangents(LSLPath, ViableCSCPaths);

		// calculate CCC paths
		FCCCPath CommonCCCBasePath;
		CommonCCCBasePath.StartPoint = StartLocation2D;
		CommonCCCBasePath.StartDirection = StartDirection2D;
		CommonCCCBasePath.EndPoint = EndLocation2D;
		CommonCCCBasePath.EndDirection = EndDirection2D;
		CommonCCCBasePath.CircleRadius = MaxTurnRadius;

		FCCCPath RLRPath = CommonCCCBasePath;
		RLRPath.StartCircleCenter = CircleCenterStartRight;
		RLRPath.IsStartCircleRight = true;
		RLRPath.IsMiddleCircleRight = false;
		RLRPath.EndCircleCenter = CircleCenterEndRight;
		RLRPath.IsEndCircleRight = true;

		FCCCPath LRLPath = CommonCCCBasePath;
		LRLPath.StartCircleCenter = CircleCenterStartLeft;
		LRLPath.IsStartCircleRight = false;
		LRLPath.IsMiddleCircleRight = true;
		LRLPath.EndCircleCenter = CircleCenterEndLeft;
		LRLPath.IsEndCircleRight = false;

		TArray<FCCCPath> ViableCCCPaths;
		CalculateCCCTouchPoints(RLRPath, ViableCCCPaths);
		CalculateCCCTouchPoints(LRLPath, ViableCCCPaths);

		bool IsShortestPathCSC = false;
		int ShortestPathIndex = -1;
		float ShortestLength;
		
		for (int32 i = 0; i < ViableCSCPaths.Num(); ++i)
		{
			float Length = CalculateCSCPathLength(ViableCSCPaths[i]);
			if (ShortestPathIndex == -1 || Length < ShortestLength)
			{
				ShortestLength = Length;
				ShortestPathIndex = i;
				IsShortestPathCSC = true;
			}
		}

		for (int32 i = 0; i < ViableCCCPaths.Num(); ++i)
		{
			float Length = CalculateCCCPathLength(ViableCCCPaths[i]);
			if (ShortestPathIndex == -1 || Length < ShortestLength)
			{
				ShortestLength = Length;
				ShortestPathIndex = i;
				IsShortestPathCSC = false;
			}
		}

		if (ShortestPathIndex == -1)
		{
			return;
		}
		
		TArray<FVector2D> PathPoints;

		if (IsShortestPathCSC)
		{
			PathPoints = CalculateSplinePathFromCSCPath(ViableCSCPaths[ShortestPathIndex]);
			
		} else
		{
			PathPoints = CalculateSplinePathFromCCCPath(ViableCCCPaths[ShortestPathIndex]);
		}
		
		float TotalDistance2D = 0.0f;
		for (int32 i = 0; i < PathPoints.Num() - 1; ++i)
		{
			TotalDistance2D += FVector2D::Distance(PathPoints[i], PathPoints[i + 1]);
		}
		float StartZ = StartLocation.Z;
		float TargetZ = TargetLocation.Z;
		
		CurrentSplinePath.Empty();

		float CummDistance = 0.0f;
		for (int i = 0; i < PathPoints.Num(); ++i)
		{
			if (i > 0)
			{
				CummDistance += FVector2D::Distance(PathPoints[i - 1], PathPoints[i]);
			}
			CurrentSplinePath.Emplace(PathPoints[i].X, PathPoints[i].Y, StartZ + (TargetZ - StartZ) * (CummDistance / TotalDistance2D));
		}
		ConveyorSplineCreatorComponent->SetSplinePoints(CurrentSplinePath, ESplineCoordinateSpace::World);
		ConveyorSplineCreatorComponent->SetTangentAtSplinePoint(0, StartDirection * EndpointTangentSize, ESplineCoordinateSpace::World);
		ConveyorSplineCreatorComponent->SetTangentAtSplinePoint(ConveyorSplineCreatorComponent->GetNumberOfSplinePoints() - 1, EndDirection * EndpointTangentSize, ESplineCoordinateSpace::World);
	}

	OverlappedComponents.Empty();
	if (AreConnectorDirectionsValid())
	{
		ConveyorSplineCreatorComponent->SpawnSpline(ValidMaterials);
	} else
	{
		ConveyorSplineCreatorComponent->SpawnSpline(InvalidMaterials);
	}

	for (USplineMeshComponent* SplineMeshComponent : ConveyorSplineCreatorComponent->GetSpawnedSplineMeshes())
	{
		SplineMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &ABuildingPreviewSplineBase::OnSplineMeshBeginOverlap);
		SplineMeshComponent->SetGenerateOverlapEvents(true);
		SplineMeshComponent->OnComponentEndOverlap.AddDynamic(this, &ABuildingPreviewSplineBase::OnSplineMeshEndOverlap);
		SplineMeshComponent->UpdateOverlaps();
	}
}

void ABuildingPreviewSplineBase::LeftMouseClicked(APlayerCharacter* PlayerCharacter)
{
	if (!bStartPlaced)
	{
		if (StartConnectionPoint == nullptr)
		{
			return;
		}
		StartLocation = StartConnectionPoint->GetActorLocation();
		StartDirection = StartConnectionPoint->GetConnectingDirection();
		bStartPlaced = true;
		EndDirection = StartDirection;
		
	} else
	{
		if (!AreConnectorDirectionsValid())
		{
			return;
		}
		
		PlayerCharacter->BuildSplineCmd(StartConnectionPoint, EndConnectionPoint, CurrentSplinePath, StartDirection * EndpointTangentSize, EndDirection * EndpointTangentSize);
		Reset();
	}
}

bool ABuildingPreviewSplineBase::AreConnectorDirectionsValid()
{
	//return StartConnectionPoint == nullptr || EndConnectionPoint == nullptr || StartConnectionPoint->FlowsOut() != EndConnectionPoint->FlowsOut();
	return true;
}

void ABuildingPreviewSplineBase::Reset()
{
	bStartPlaced = false;
	StartConnectionPoint = nullptr;
	EndConnectionPoint = nullptr;
	ConveyorSplineCreatorComponent->ClearSplinePoints();
	bIsColliding = false;
	LastDesiredPlacementConfig.Valid = false;
}

float ABuildingPreviewSplineBase::GetAngleOnCircle(FVector2D CircleCenter, FVector2D PointOnCircle)
{
	FVector2D PointOnZeroOriginCircle = PointOnCircle - CircleCenter;
	float Angle = FMath::Atan2(PointOnZeroOriginCircle.Y, PointOnZeroOriginCircle.X);
	return Angle < 0.0f ? Angle + 2 * PI : Angle;
}

void ABuildingPreviewSplineBase::CalculateCSCTangents(const FCSCPath& CSCPath, TArray<FCSCPath>& ViableCSCPaths)
{
	FVector2D CentersDirection = (CSCPath.StartCircleCenter - CSCPath.EndCircleCenter).GetSafeNormal();

	TArray<TPair<FVector2D, FVector2D>> TangentLocations;

	// external tangent 1 - point is calculated as circle center + a vector
	// perpendicular to the the CentersDirection with components (-y, x)  
	FVector2D Circle1TangentPoint1 = CSCPath.StartCircleCenter + FVector2D(-CentersDirection.Y, CentersDirection.X) * CSCPath.CircleRadius;
	FVector2D Circle2TangentPoint1 = CSCPath.EndCircleCenter + FVector2D(-CentersDirection.Y, CentersDirection.X) * CSCPath.CircleRadius;
	TangentLocations.Emplace(Circle1TangentPoint1, Circle2TangentPoint1);
	
	// external tangent 2 - point is calculated as circle center + a vector
	// perpendicular to the the CentersDirection with components (y, -x)
	FVector2D Circle1TangentPoint2 = CSCPath.StartCircleCenter + FVector2D(CentersDirection.Y, -CentersDirection.X) * CSCPath.CircleRadius;
	FVector2D Circle2TangentPoint2 = CSCPath.EndCircleCenter + FVector2D(CentersDirection.Y, -CentersDirection.X) * CSCPath.CircleRadius;
	TangentLocations.Emplace(Circle1TangentPoint2, Circle2TangentPoint2);

	// internal tangent lines are valid only if the two circles don't intersect
	if (FVector2D::Distance(CSCPath.StartCircleCenter, CSCPath.EndCircleCenter) > 2 * CSCPath.CircleRadius)
	{
		// find internal center, middle point between the 2 circle centers
		// internal center == external center (the point where external tangents intersect)
		// if circles have the same radius
		FVector2D InternalCenter = (CSCPath.StartCircleCenter + CSCPath.EndCircleCenter) / 2;
		
		// get distance from circle center to the internal center, the distance is same for both circles
		float InternalCenter_CircleCenterDist = FVector2D::Distance(InternalCenter, CSCPath.StartCircleCenter);

		// calculate angle between tangent line and internal center line
		float InternalAngle_InternalCenter_CircleCenter = FMath::Asin(CSCPath.CircleRadius / InternalCenter_CircleCenterDist);

		float InternalTangentHalfLength = FMath::Sqrt(FMath::Square(InternalCenter_CircleCenterDist) - FMath::Square(CSCPath.CircleRadius));

		FVector2D Circle1CenterDir = (CSCPath.StartCircleCenter - InternalCenter).GetSafeNormal();
		FVector2D Circle2CenterDir = (CSCPath.EndCircleCenter - InternalCenter).GetSafeNormal();
		
		// internal tangent 1, calculated with positive rotation from the internal center line
		FVector2D Circle1TangentPoint3 = InternalCenter + Circle1CenterDir.GetRotated(InternalAngle_InternalCenter_CircleCenter * 180.0f / PI) * InternalTangentHalfLength;
		FVector2D Circle2TangentPoint3 = InternalCenter + Circle2CenterDir.GetRotated(InternalAngle_InternalCenter_CircleCenter * 180.0f / PI) * InternalTangentHalfLength;
		TangentLocations.Emplace(Circle1TangentPoint3, Circle2TangentPoint3);

		//Internal tangent 2, calculated with negative rotation from the internal center line
		FVector2D Circle1TangentPoint4 = InternalCenter + Circle1CenterDir.GetRotated(-InternalAngle_InternalCenter_CircleCenter * 180.0f / PI) * InternalTangentHalfLength;
		FVector2D Circle2TangentPoint4 = InternalCenter + Circle2CenterDir.GetRotated(-InternalAngle_InternalCenter_CircleCenter * 180.0f / PI) * InternalTangentHalfLength;
		TangentLocations.Emplace(Circle1TangentPoint4, Circle2TangentPoint4);
	}

	float StartPointAngleOnStartCircle = GetAngleOnCircle(CSCPath.StartCircleCenter, CSCPath.StartPoint);
	float EndPointAngleOnEndCircle = GetAngleOnCircle(CSCPath.EndCircleCenter, CSCPath.EndPoint);
	
	for (auto TangentLocation : TangentLocations)
	{
		float StartTangentPointAngle = GetAngleOnCircle(CSCPath.StartCircleCenter, TangentLocation.Key); 
		float TargetTangentPointAngle = GetAngleOnCircle(CSCPath.EndCircleCenter, TangentLocation.Value);

		FVector2D RotatedStartDirection2D = CSCPath.StartDirection.GetRotated((StartTangentPointAngle - StartPointAngleOnStartCircle) * 180.0f / PI);
		FVector2D RotatedTargetDirection2D = CSCPath.EndDirection.GetRotated((TargetTangentPointAngle - EndPointAngleOnEndCircle) * 180.0f / PI);

		if (IsOnRightSideOfLine(RotatedStartDirection2D, TangentLocation.Key, TangentLocation.Value)
			&& !IsOnRightSideOfLine(RotatedTargetDirection2D, TangentLocation.Value, TangentLocation.Key))
		{
			ViableCSCPaths.Add(CSCPath);
			ViableCSCPaths.Last().StartCircleTangentPoint = TangentLocation.Key;
			ViableCSCPaths.Last().EndCircleTangentPoint = TangentLocation.Value;
			return;
		}
	}
}

void ABuildingPreviewSplineBase::CalculateCCCTouchPoints(const FCCCPath& CCCPath, TArray<FCCCPath>& ViableCCCPaths)
{
	if (FVector2D::Distance(CCCPath.StartPoint, CCCPath.EndPoint) > 4 * CCCPath.CircleRadius)
	{
		return;
	}

	ViableCCCPaths.Add(CalculateMidCircle(CCCPath, 1));
	ViableCCCPaths.Add(CalculateMidCircle(CCCPath, -1));
	
}

inline ABuildingPreviewSplineBase::FCCCPath ABuildingPreviewSplineBase::CalculateMidCircle(const FCCCPath& CCCPath,
	int Direction)
{
	FCCCPath ResultCCCPath = CCCPath;
	
	float CirclesDist = FVector2D::Distance(ResultCCCPath.StartCircleCenter, ResultCCCPath.EndCircleCenter);
	float Angle = FMath::Acos(FMath::Square(CirclesDist) / (2 * 2 * ResultCCCPath.CircleRadius * CirclesDist));

	FVector2D CirclesNormal = (ResultCCCPath.EndCircleCenter - ResultCCCPath.StartCircleCenter).GetSafeNormal();
	
	float AngleInDeg = Angle * (180 / PI) * Direction;

	FVector2D RotatedCirclesNormal = CirclesNormal.GetRotated(AngleInDeg);

	ResultCCCPath.MiddleCircleCenter = ResultCCCPath.StartCircleCenter + RotatedCirclesNormal * 2 * ResultCCCPath.CircleRadius;
	ResultCCCPath.StartCircleTouchPoint = ResultCCCPath.StartCircleCenter + RotatedCirclesNormal * ResultCCCPath.CircleRadius;

	FVector2D MidCenterToEndCenterNormal = (ResultCCCPath.EndCircleCenter - ResultCCCPath.MiddleCircleCenter).GetSafeNormal();
	ResultCCCPath.EndCircleTouchPoint = ResultCCCPath.MiddleCircleCenter + MidCenterToEndCenterNormal * ResultCCCPath.CircleRadius;

	return ResultCCCPath;
}

bool ABuildingPreviewSplineBase::DoCirclesIntersect(FVector2D Circle1Center, FVector2D Circle2Center,
	float CircleRadius)
{
	return FVector2D::DistSquared(Circle1Center, Circle2Center) <= 4 * CircleRadius * CircleRadius;
}

void ABuildingPreviewSplineBase::OnSplineMeshBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UIndexSplineMeshComponent* ThisOverlappedSplineMeshComponent = Cast<UIndexSplineMeshComponent>(OverlappedComponent);
	check(ThisOverlappedSplineMeshComponent);
	
	UIndexSplineMeshComponent* OtherOverlappedSplineMeshComponent = Cast<UIndexSplineMeshComponent>(OtherComp);
	
	if (OtherActor == this && OtherOverlappedSplineMeshComponent != nullptr && FMath::Abs(OtherOverlappedSplineMeshComponent->Id - ThisOverlappedSplineMeshComponent->Id) <= 1)
	{
		return;
	}

	/*
	if (ThisOverlappedSplineMeshComponent->Id == 0  && StartConnectionPoint != nullptr && StartConnectionPoint->GetParentActor() == OtherActor)
	{
		return;
	}
	if (ThisOverlappedSplineMeshComponent->bIsLast && EndConnectionPoint != nullptr && EndConnectionPoint->GetParentActor() == OtherActor)
	{
		return;
	}
	
	OverlappedComponents.Add(TPair<UPrimitiveComponent*, UPrimitiveComponent*>{OverlappedComponent, OtherComp});
	if (OverlappedComponents.Num() == 1)
	{
		ConveyorSplineCreatorComponent->SetMaterials(InvalidMaterials);
	}
}

void ABuildingPreviewSplineBase::OnSplineMeshEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == this)
	{
		return;
	}
	OverlappedComponents.Remove(TPair<UPrimitiveComponent*, UPrimitiveComponent*>{OverlappedComponent, OtherComp});
	if (OverlappedComponents.Num() == 0)
	{
		ConveyorSplineCreatorComponent->SetMaterials(ValidMaterials);
	}
}

TArray<FVector2D> ABuildingPreviewSplineBase::CalculateSplinePathFromCSCPath(const FCSCPath& CSCPath)
{
	float PathLength = CalculateCSCPathLength(CSCPath);
	float SplinePathDistBetweenPoints = CalculateBestDistBetweenPathPoints(PathLength);
	
	TArray<FVector2D> PathPoints;
	
	float InitDistToLineFirstPoint = AddPointsOnCircle(CSCPath.StartCircleCenter, CSCPath.CircleRadius, CSCPath.StartPoint, CSCPath.StartCircleTangentPoint, CSCPath.IsStartCircleRight, SplinePathDistBetweenPoints, 0.0f, false, PathPoints);

	float DistAlongNormal = InitDistToLineFirstPoint;
	
	float LineLength = FVector2D::Distance(CSCPath.StartCircleTangentPoint, CSCPath.EndCircleTangentPoint);
	FVector2D LineDirection = (CSCPath.EndCircleTangentPoint - CSCPath.StartCircleTangentPoint).GetSafeNormal();
	for (;;)
	{
		if (DistAlongNormal > LineLength)
		{
			break;
		}
		PathPoints.Add(LineDirection * DistAlongNormal + CSCPath.StartCircleTangentPoint);
		DistAlongNormal += SplinePathDistBetweenPoints;
	}

	float InitDistToFirstCirclePoint = DistAlongNormal - LineLength; 
	AddPointsOnCircle(CSCPath.EndCircleCenter, CSCPath.CircleRadius, CSCPath.EndCircleTangentPoint, CSCPath.EndPoint, CSCPath.IsEndCircleRight, SplinePathDistBetweenPoints, InitDistToFirstCirclePoint, true, PathPoints);
	
	return PathPoints;
}

TArray<FVector2D> ABuildingPreviewSplineBase::CalculateSplinePathFromCCCPath(const FCCCPath& CCCPath)
{
	float PathLength = CalculateCCCPathLength(CCCPath);
	if (PathLength <= 0.0f)
	{
		return {};
	}
	float SplinePathDistBetweenPoints = CalculateBestDistBetweenPathPoints(PathLength);
	
	TArray<FVector2D> PathPoints;
	float InitDistToFirstPoint = AddPointsOnCircle(CCCPath.StartCircleCenter, CCCPath.CircleRadius,
		CCCPath.StartPoint, CCCPath.StartCircleTouchPoint, CCCPath.IsStartCircleRight,
		SplinePathDistBetweenPoints, 0.0f, false, PathPoints);

	InitDistToFirstPoint = AddPointsOnCircle(CCCPath.MiddleCircleCenter, CCCPath.CircleRadius,
		CCCPath.StartCircleTouchPoint, CCCPath.EndCircleTouchPoint, CCCPath.IsMiddleCircleRight,
		SplinePathDistBetweenPoints, InitDistToFirstPoint, false, PathPoints);

	AddPointsOnCircle(CCCPath.EndCircleCenter, CCCPath.CircleRadius, CCCPath.EndCircleTouchPoint,
		CCCPath.EndPoint, CCCPath.IsEndCircleRight, SplinePathDistBetweenPoints, InitDistToFirstPoint,
	true, PathPoints);

	return PathPoints;
}

float ABuildingPreviewSplineBase::CalculateCCCPathLength(const FCCCPath& CCCPath)
{
	float StartCircleSegmentLength = GetCircleSegmentLength(CCCPath.StartCircleCenter, CCCPath.StartPoint, CCCPath.StartCircleTouchPoint, CCCPath.IsStartCircleRight, CCCPath.CircleRadius);
	float MidCircleSegmentLength = GetCircleSegmentLength(CCCPath.MiddleCircleCenter, CCCPath.StartCircleTouchPoint, CCCPath.EndCircleTouchPoint, CCCPath.IsMiddleCircleRight, CCCPath.CircleRadius);
	float EndCircleSegmentLength = GetCircleSegmentLength(CCCPath.EndCircleCenter, CCCPath.EndCircleTouchPoint, CCCPath.EndPoint, CCCPath.IsEndCircleRight, CCCPath.CircleRadius);
	return StartCircleSegmentLength + MidCircleSegmentLength + EndCircleSegmentLength;
}

bool ABuildingPreviewSplineBase::IsOnRightSideOfLine(FVector2D LineNormal, FVector2D LinePoint, FVector2D TestPoint)
{
	float A = LineNormal.X;
	float B = LineNormal.Y;
	float C = -LinePoint.X * LineNormal.X - LinePoint.Y * LineNormal.Y;
	return A * TestPoint.X + B * TestPoint.Y + C >= 0;
}

float ABuildingPreviewSplineBase::AddPointsOnCircle(FVector2D CircleCenter, float CircleRadius, FVector2D StartPoint,
	FVector2D EndPoint, bool IsCircleRight, float SplinePathDistBetweenPoints, float InitDistToFirstPoint,
	bool IsLastInPath, TArray<FVector2D>& OutPathPoints)
{
	const float AngleIncrement = SplinePathDistBetweenPoints / CircleRadius;
	const float InitAngleDistToFirstPoint = InitDistToFirstPoint / CircleRadius;

	const float StartAngle = GetAngleOnCircle(CircleCenter, StartPoint);
	const float EndAngle = GetAngleOnCircle(CircleCenter, EndPoint);

	float AngleDiff = GetCirclePointsAngleDifference(StartAngle, EndAngle, IsCircleRight);
	if (!IsCircleRight)
	{
		float AngleIt = InitAngleDistToFirstPoint;
		for (;;)
		{
			if (AngleIt > AngleDiff)
			{
				if (IsLastInPath && AngleIt - AngleDiff < AngleIncrement / 2)
				{
					OutPathPoints.Add(GetPointOnCircle(CircleCenter, CircleRadius, StartAngle + AngleDiff));
				}
				
				return (AngleIt - AngleDiff) * CircleRadius;
			}
			
			float CircleAngle = StartAngle + AngleIt; 
			OutPathPoints.Add(GetPointOnCircle(CircleCenter, CircleRadius, CircleAngle));
			AngleIt += AngleIncrement;
		}
	}
		
	float AngleIt = -InitAngleDistToFirstPoint;
	for (;;)
	{
		if (AngleIt < AngleDiff)
		{
				if (IsLastInPath && FMath::Abs((AngleIt - AngleDiff)) < AngleIncrement / 2)
				{
					OutPathPoints.Add(GetPointOnCircle(CircleCenter, CircleRadius, StartAngle + AngleDiff));
				}
			return (AngleDiff - AngleIt) * CircleRadius;
		}
		float CircleAngle = StartAngle + AngleIt; 
		OutPathPoints.Add(GetPointOnCircle(CircleCenter, CircleRadius, CircleAngle));
		AngleIt -= AngleIncrement;
	}
}

inline float ABuildingPreviewSplineBase::CalculateCSCPathLength(const FCSCPath& CSCPath)
{
	float StartCircleLength = GetCircleSegmentLength(CSCPath.StartCircleCenter, CSCPath.StartPoint, CSCPath.StartCircleTangentPoint, CSCPath.IsStartCircleRight, CSCPath.CircleRadius);
	
	float LineLength = FVector2D::Distance(CSCPath.StartCircleTangentPoint, CSCPath.EndCircleTangentPoint);

	float EndCircleLength = GetCircleSegmentLength(CSCPath.EndCircleCenter, CSCPath.EndCircleTangentPoint, CSCPath.EndPoint, CSCPath.IsEndCircleRight, CSCPath.CircleRadius);

	return StartCircleLength + LineLength + EndCircleLength;
}

inline float ABuildingPreviewSplineBase::GetCirclePointsAngleDifference(float StartAngle, float EndAngle, bool IsCircleRight)
{
	float AngleDiff = EndAngle - StartAngle;
	if (IsCircleRight)
	{
		if (AngleDiff > 0.0f)
		{
			AngleDiff -= 2 * PI;
		}
	} else
	{
		if (AngleDiff < 0.0f)
		{
			AngleDiff += 2 * PI;
		}
	}
	return AngleDiff;
}

inline float ABuildingPreviewSplineBase::CalculateBestDistBetweenPathPoints(float PathLength)
{
	int32 DesiredSplineSegments = FMath::RoundToInt( PathLength / DesiredSplinePathDistBetweenPoints);
	return PathLength / DesiredSplineSegments;
}

FVector2D ABuildingPreviewSplineBase::GetPointOnCircle(FVector2D CircleCenter, float CircleRadius, float Angle)
{
	return FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * CircleRadius + CircleCenter;
}

float ABuildingPreviewSplineBase::GetCircleSegmentLength(FVector2D CircleCenter, FVector2D StartPoint, FVector2D EndPoint,
	bool IsCircleRight, float CircleRadius)
{
	float StartAngle = GetAngleOnCircle(CircleCenter, StartPoint);
	float EndAngle = GetAngleOnCircle(CircleCenter, EndPoint);

	return FMath::Abs(GetCirclePointsAngleDifference(StartAngle, EndAngle, IsCircleRight)) * CircleRadius;
}*/