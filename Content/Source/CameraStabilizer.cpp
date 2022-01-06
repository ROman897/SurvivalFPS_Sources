// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraStabilizer.h"


// Sets default values for this component's properties
UCameraStabilizer::UCameraStabilizer()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCameraStabilizer::BeginPlay()
{
	Super::BeginPlay();

	if (ChildCamera != nullptr)
	{
		ChildCamera->SetWorldLocation(GetComponentLocation());
	}
}


// Called every frame
void UCameraStabilizer::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ChildCamera == nullptr)
	{
		return;
	}

	if (!bStabilize)
	{
		ChildCamera->SetWorldLocation(GetComponentLocation());
		return;
	}

	FVector DiffVector = GetComponentLocation() - ChildCamera->GetComponentLocation();
	FVector DiffDirection = DiffVector.GetSafeNormal();
	if (DiffDirection == FVector::ZeroVector)
	{
		return;
	}

	float DiffDist = DiffVector.Size();
	float DistToMove = FMath::Min(DeltaTime * StabilizationSpeed, DiffDist);
	ChildCamera->SetWorldLocation(ChildCamera->GetComponentLocation() + DiffDirection * DistToMove);

	FRotator RotationDiff = GetComponentRotation() - ChildCamera->GetComponentRotation();

}

void UCameraStabilizer::SetCamera(USceneComponent* ChildComponent)
{
	ChildCamera = ChildComponent;
}

