// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPartFloor.h"


// Sets default values
AShipPartFloor::AShipPartFloor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMeshComp"));
	FloorMesh->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AShipPartFloor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AShipPartFloor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

