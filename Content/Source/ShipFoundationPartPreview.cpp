// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipFoundationPartPreview.h"

#include "CollisionTraceChannels.h"
#include "IngameGameState.h"
#include "PlayerCharacter.h"
#include "Ship.h"


// Sets default values
AShipFoundationPartPreview::AShipFoundationPartPreview()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRootComp"));
	SetRootComponent(SceneRoot);
	
	PreviewMeshBase = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMeshBaseComp"));
	PreviewMeshBase->SetupAttachment(GetRootComponent());
	PreviewMeshBase->SetCollisionProfileName("NoCollision");

	PreviewCollider = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewColliderComp"));
	PreviewCollider->SetCollisionProfileName("ShipPartPreview");
	PreviewCollider->SetupAttachment(GetRootComponent());
	PreviewCollider->SetHiddenInGame(true);
}

// Called when the game starts or when spawned
void AShipFoundationPartPreview::BeginPlay()
{
	Super::BeginPlay();
	
	PreviewCollider->OnComponentBeginOverlap.AddDynamic(this, &AShipFoundationPartPreview::OnPreviewColliderBeginOverlap);
	PreviewCollider->OnComponentEndOverlap.AddDynamic(this, &AShipFoundationPartPreview::OnPreviewColliderEndOverlap);
	PreviewCollider->UpdateOverlaps();
	
}

void AShipFoundationPartPreview::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (AShip* Ship : GetWorld()->GetGameState<AIngameGameState>()->AllPlayerShips)
	{
		Ship->DisableSnapsOfType();
	}
}

// Called every frame
void AShipFoundationPartPreview::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AShipFoundationPartPreview::SetDesiredPlacement(FVector CameraLocation, FVector CameraForward,
	int32 ZAxisRotationPresses)
{
	for (AShip* Ship : GetWorld()->GetGameState<AIngameGameState>()->AllPlayerShips)
	{
		Ship->EnableSnapsOfType(ShipPartDataAsset->ShipPartSnapType, CameraLocation, MaxDistanceFromPlayer);
	}
	
	SetDesiredPlacementWithSnap(CameraLocation, CameraForward, ZAxisRotationPresses);
	UpdatePreviewMaterial();
}

void AShipFoundationPartPreview::LeftMouseClicked(APlayerCharacter* PlayerCharacter)
{
	if (CanBuild())
	{
		CurrentSerializedShipFoundationPartData.ShipPartDataAsset = ShipPartDataAsset;
		// CanBuild() == true implies that CurrentSerializedShipFoundationPartData is valid
		PlayerCharacter->BuildFoundationShipPartCmd(CurrentSnapShip, CurrentSerializedShipFoundationPartData);
	}
}

bool AShipFoundationPartPreview::CanBuild()
{
	// bHitValidSnap implies HitShip is valid
	return CurrentOverlapCount <= 0 && bHitValidSnap;
}

void AShipFoundationPartPreview::OnPreviewColliderBeginOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	++CurrentOverlapCount;
	UpdatePreviewMaterial();
}

void AShipFoundationPartPreview::OnPreviewColliderEndOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	--CurrentOverlapCount;
	UpdatePreviewMaterial();
}

void AShipFoundationPartPreview::UpdatePreviewMaterial()
{
	if (CanBuild())
	{
		PreviewMeshBase->SetMaterial(0, PreviewValidMaterial);
		
	} else
	{
		PreviewMeshBase->SetMaterial(0, PreviewInvalidMaterial);
	}
}

void AShipFoundationPartPreview::SetDesiredPlacementWithSnap(FVector CameraLocation, FVector CameraForward,
	int32 ZAxisRotationPresses)
{
	// this is just for previewing when we didn't hit anything
	const FRotator DesiredFreeformRotation(0, ZAxisRotationPresses * NonSnapZRotationPerKeyPress, 0);
	
	bHitValidSnap = false;

	// we get a bunch of snap colliders that we hit + a possible blocking hit at the last index
	// we need to filter out snap colliders of incorrect snap type
	TArray<FHitResult> HitResults = LineTraceMultiMaxDistFromPlayer(CameraLocation, CameraForward, TRACE_CHANNEL_BUILDING_SNAP);

	struct FSnapHit
	{
		AShip* HitShip;
		FShipPartSnapHitResult ShipPartSnapHitResult;
		FHitResult HitResult;
	};

	TArray<FSnapHit> ViableSnapHits;

	float ViableSnapClosestDistToCenter = 0.0f;
	int ClosestViableSnapIndex = 0;

	for (const FHitResult& HitResult : HitResults)
	{
		if (HitResult.bBlockingHit)
		{
			// this is the last hit cause it's blocking
			if (ViableSnapHits.Num() > 0)
			{
				// we hit some valid snaps before hitting a blocking hit, so snap to some of them
				break;
			}
			
			// we didn't hit any valid snaps,
			// so set preview location to hit location
			SetActorLocation(HitResult.Location);
			
			AShip* HitShip = AShip::FindParentShip(HitResult.GetActor());

			if (HitShip != nullptr)
			{
				// if we hit something on a moving ship but not a snap collider,
				//e.g. floor, we want to move this preview with the ship
				
				AttachToActor(HitShip, FAttachmentTransformRules::KeepWorldTransform);
				SetActorRelativeRotation(DesiredFreeformRotation);
				SetActorRelativeScale3D(FVector::OneVector);
				
			} else
			{
				// we hit something which isn't on a ship, so don't attach to anything
				DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				SetActorRotation(DesiredFreeformRotation);
				SetActorRelativeScale3D(FVector::OneVector);
			}
			return;
		}

		// now we know this is an overlap hit, so we assume and check that we hit a ship
		AShip* HitShip = Cast<AShip>(HitResult.GetActor());
		check(HitShip != nullptr);

		FShipPartSnapHitResult ShipPartSnapHitResult = HitShip->EvaluateSnapHit(HitResult);
		if (ShipPartSnapHitResult.ShipPartSnapHandle.ShipPartSnapType == static_cast<int>(ShipPartDataAsset->ShipPartSnapType))
		{
			float DistToSnapCenter = FMath::PointDistToLine(HitResult.GetComponent()->GetComponentLocation(), CameraForward, CameraLocation);
			if (ViableSnapHits.Num() == 0 || DistToSnapCenter < ViableSnapClosestDistToCenter)
			{
				ViableSnapClosestDistToCenter = DistToSnapCenter;
				ClosestViableSnapIndex = ViableSnapHits.Num();
			}
			ViableSnapHits.Emplace(FSnapHit {HitShip, ShipPartSnapHitResult, HitResult});
		}
	}

	if (ViableSnapHits.Num() == 0)
	{
		// there's no blocking hit and we didn't hit any snap collider
		// so just move target at the end of range where player's looking
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		SetActorLocation(CameraLocation + CameraForward * MaxDistanceFromPlayer);
		SetActorRotation(DesiredFreeformRotation);
		SetActorRelativeScale3D(FVector::OneVector);
		return;
	}

	// if we're here, we know that there's at least one viable snap, we just need to choose best one
	// based on distance from center and desired rotation

	FVector MostViableSnapImpactPoint = ViableSnapHits[ClosestViableSnapIndex].HitResult.Location;
	
	CurrentSnapShip = ViableSnapHits[ClosestViableSnapIndex].HitShip;
	bHitValidSnap = true;

	TArray<FShipPartSnapHitResult> SelectedSnapPoints;

	for (const FSnapHit& ViableSnapHit : ViableSnapHits)
	{
		if (ViableSnapHit.HitShip != CurrentSnapShip)
		{
			continue;
		}
		if (FVector::DistSquared(ViableSnapHit.HitResult.Location, MostViableSnapImpactPoint) < 1.0f)
		{
			SelectedSnapPoints.Add(ViableSnapHit.ShipPartSnapHitResult);
		}
	}
	check(SelectedSnapPoints.Num() > 0);

	SelectedSnapPoints.Sort([] (const FShipPartSnapHitResult Lhs, const FShipPartSnapHitResult& Rhs)
	{
		check(Lhs.ShipPartSnapHandle.TileCoords != Rhs.ShipPartSnapHandle.TileCoords
			|| Lhs.ShipPartSnapHandle.SnapId != Rhs.ShipPartSnapHandle.SnapId);

		return Tie(Lhs.ShipPartSnapHandle.SnapId, Lhs.ShipPartSnapHandle.TileCoords.X, Lhs.ShipPartSnapHandle.TileCoords.Y, Lhs.ShipPartSnapHandle.TileCoords.Z)
			< Tie(Rhs.ShipPartSnapHandle.SnapId, Rhs.ShipPartSnapHandle.TileCoords.X, Rhs.ShipPartSnapHandle.TileCoords.Y, Rhs.ShipPartSnapHandle.TileCoords.Z);
	});
	
	int ChosenSnapPointIndex = (ZAxisRotationPresses % SelectedSnapPoints.Num() + SelectedSnapPoints.Num()) % SelectedSnapPoints.Num();
	const FShipPartSnapHitResult& SelectedSnapPoint = SelectedSnapPoints[ChosenSnapPointIndex];

	AttachToActor(CurrentSnapShip, FAttachmentTransformRules::KeepWorldTransform);
	SetActorRelativeLocation(SelectedSnapPoint.RelativeSnapLocation);
	SetActorRelativeRotation(SelectedSnapPoint.RelativeSnapRotation);
	SetActorRelativeScale3D(SelectedSnapPoint.RelativeSnapScale);

	CurrentSerializedShipFoundationPartData.ShipTileCoords = SelectedSnapPoint.ShipPartSnapHandle.TileCoords;
	CurrentSerializedShipFoundationPartData.TileSnapId = SelectedSnapPoint.ShipPartSnapHandle.SnapId;
}

