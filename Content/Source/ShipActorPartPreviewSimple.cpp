// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipActorPartPreviewSimple.h"

#include "CollisionTraceChannels.h"
#include "PlayerCharacter.h"
#include "Ship.h"

// Sets default values
AShipActorPartPreviewSimple::AShipActorPartPreviewSimple()
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
void AShipActorPartPreviewSimple::BeginPlay()
{
	Super::BeginPlay();
	
	PreviewCollider->OnComponentBeginOverlap.AddDynamic(this, &AShipActorPartPreviewSimple::OnPreviewColliderBeginOverlap);
	PreviewCollider->OnComponentEndOverlap.AddDynamic(this, &AShipActorPartPreviewSimple::OnPreviewColliderEndOverlap);
	PreviewCollider->UpdateOverlaps();
}

// Called every frame
void AShipActorPartPreviewSimple::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AShipActorPartPreviewSimple::SetDesiredPlacement(FVector CameraLocation, FVector CameraForward,
	int32 ZAxisRotationPresses)
{
	SetDesiredPlacementLocation(CameraLocation, CameraForward, ZAxisRotationPresses);
	UpdatePreviewMaterial();
}

void AShipActorPartPreviewSimple::SetDesiredPlacementLocation(FVector CameraLocation, FVector CameraForward,
	int32 ZAxisRotationPresses)
{
	const FRotator DesiredZRotation(0, ZAxisRotationPresses * ZRotationPerKeyPress, 0);

	FHitResult TargetHit = LineTraceMaxDistFromPlayer(CameraLocation, CameraForward, TRACE_CHANNEL_SOLID_MATTER);

	// if we didn't hit anything, just set location to max range
	if (TargetHit.GetActor() == nullptr)
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		SetActorLocation(CameraLocation + CameraForward * MaxDistanceFromPlayer);
		SetActorRotation(DesiredZRotation);
		return;
	}
	
	SetActorLocation(TargetHit.Location);
	
	HitShip = Cast<AShip>(TargetHit.Actor);

	AShip* AttachToShip = HitShip;
	if (AttachToShip == nullptr)
	{
		// even if we didn't directly hit ship mesh, we might've hit an actor that is on the ship
		// in which case we want to move this preview with the ship
		AttachToShip = AShip::FindParentShip(TargetHit.GetActor());
	}

	if (AttachToShip != nullptr)
	{
		AttachToActor(HitShip, FAttachmentTransformRules::KeepWorldTransform);
		SetActorRelativeRotation(DesiredZRotation);
	} else
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		SetActorRotation(DesiredZRotation);
	}
}

void AShipActorPartPreviewSimple::LeftMouseClicked(APlayerCharacter* PlayerCharacter)
{
	if (CanBuild())
	{
		PlayerCharacter->BuildShipActorPartCmd(HitShip, GetActorLocation(), GetActorRotation());
	}
}

bool AShipActorPartPreviewSimple::CanBuild()
{
	return CurrentOverlapCount <= 0 && HitShip != nullptr;
}

void AShipActorPartPreviewSimple::OnPreviewColliderBeginOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	++CurrentOverlapCount;
	UpdatePreviewMaterial();
}

void AShipActorPartPreviewSimple::OnPreviewColliderEndOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	--CurrentOverlapCount;
	UpdatePreviewMaterial();
}

void AShipActorPartPreviewSimple::UpdatePreviewMaterial()
{
	if (CanBuild())
	{
		PreviewMeshBase->SetMaterial(0, PreviewValidMaterial);
		
	} else
	{
		PreviewMeshBase->SetMaterial(0, PreviewInvalidMaterial);
	}
}

void AShipActorPartPreviewSimple::HitNothing(FVector CameraLocation, FVector CameraForward, const FRotator& DesiredFreeformRotation)
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorLocation(CameraLocation + CameraForward * MaxDistanceFromPlayer);
	SetActorRotation(DesiredFreeformRotation);
}

