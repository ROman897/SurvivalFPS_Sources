// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPartPreviewBase.h"


AShipPartPreviewBase::AShipPartPreviewBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PreviewValidMaterialObj(TEXT("MaterialInstanceConstant'/Game/Materials/BuildingPreviewValidMaterialInstance.BuildingPreviewValidMaterialInstance'"));
	PreviewValidMaterial = PreviewValidMaterialObj.Object;
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PreviewInvalidMaterialObj(TEXT("MaterialInstanceConstant'/Game/Materials/BuildingPreviewInvalidMaterialInstance.BuildingPreviewInvalidMaterialInstance'"));
	PreviewInvalidMaterial = PreviewInvalidMaterialObj.Object;
}

// Called when the game starts or when spawned
void AShipPartPreviewBase::BeginPlay()
{
	Super::BeginPlay();

	check(PreviewValidMaterial != nullptr);
	check(PreviewInvalidMaterial != nullptr);
}

// Called every frame
void AShipPartPreviewBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AShipPartPreviewBase::SetDesiredPlacement(FVector CameraLocation, FVector CameraForward, int32 ZAxisRotationPresses)
{
	// must be implemented
	check(false);
}

FHitResult AShipPartPreviewBase::LineTrace(const FVector& StartTrace, const FVector& Direction,
	float TraceRange, ECollisionChannel CollisionChannel) const
{
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(GroundTrace), true, GetOwner());

	FVector EndTrace = StartTrace + Direction * TraceRange;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, CollisionChannel, TraceParams);

	return Hit;
}

FHitResult AShipPartPreviewBase::LineTraceMaxDistFromPlayer(const FVector& StartTrace, const FVector& Direction,
	ECollisionChannel CollisionChannel) const
{
	return LineTrace(StartTrace, Direction, MaxDistanceFromPlayer, CollisionChannel);
}

TArray<FHitResult> AShipPartPreviewBase::LineTraceMulti(const FVector& StartTrace, const FVector& Direction,
	float TraceRange, ECollisionChannel CollisionChannel)
{
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(GroundTrace), true, GetOwner());

	FVector EndTrace = StartTrace + Direction * TraceRange;

	TArray<FHitResult> HitResults;
	
	GetWorld()->LineTraceMultiByChannel(HitResults, StartTrace, EndTrace, CollisionChannel, TraceParams);
	return HitResults;
}

TArray<FHitResult> AShipPartPreviewBase::LineTraceMultiMaxDistFromPlayer(const FVector& StartTrace,
	const FVector& Direction, ECollisionChannel CollisionChannel)
{
	return LineTraceMulti(StartTrace, Direction, MaxDistanceFromPlayer, CollisionChannel);
}

void AShipPartPreviewBase::LeftMouseClicked(class APlayerCharacter*)
{
	// must be implemented
	check(false);
}

void AShipPartPreviewBase::OnMouseWheel(float Val)
{
}

