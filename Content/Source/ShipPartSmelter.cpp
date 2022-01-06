// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPartSmelter.h"


// Sets default values
AShipPartSmelter::AShipPartSmelter()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SmelterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SmelterMeshComp"));
	SmelterMesh->SetupAttachment(GetRootComponent());

	FocusablePrimitives.Add(SmelterMesh);
}
