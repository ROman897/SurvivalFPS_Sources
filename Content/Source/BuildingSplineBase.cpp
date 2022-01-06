// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildingSplineBase.h"

#include "SplineCreatorComponent.h"


// Sets default values
ABuildingSplineBase::ABuildingSplineBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ConveyorSplineCreatorComponent = CreateDefaultSubobject<USplineCreatorComponent>(TEXT("ConveyorSplineCreatorComp"));
}

// Called when the game starts or when spawned
void ABuildingSplineBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABuildingSplineBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABuildingSplineBase::SetSpline(const TArray<FVector>& SplineLocations, FVector StartTangent, FVector EndTangent, bool IsDirectionReversed)
{
	//check(SplineLocations.Num() == SplineTangents.Num());

	/*ConveyorSplineCreatorComponent->SetSplinePoints(SplineLocations, ESplineCoordinateSpace::World);
	if (StartTangent.Size() > 0)
	{
		ConveyorSplineCreatorComponent->SetTangentAtSplinePoint(0, StartTangent, ESplineCoordinateSpace::World);
	}
	if (EndTangent.Size() > 0)
	{
		ConveyorSplineCreatorComponent->SetTangentAtSplinePoint(ConveyorSplineCreatorComponent->GetNumberOfSplinePoints() - 1, EndTangent, ESplineCoordinateSpace::World);
	}

	if (IsDirectionReversed)
	{
		ConveyorSplineCreatorComponent->SpawnSpline(ReversedDirectionSplineMaterials);
	} else
	{
		ConveyorSplineCreatorComponent->SpawnSpline(NormalDirectionSplineMaterials);
		
	}
	SplineLength = ConveyorSplineCreatorComponent->GetSplineLength();
	*/
}

void ABuildingSplineBase::SetConnectors(AConnectionPoint* NewStartConnector, AConnectionPoint* NewEndConnector)
{
}

void ABuildingSplineBase::Start()
{
}

