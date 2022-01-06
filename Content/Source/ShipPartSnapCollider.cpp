// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPartSnapCollider.h"

#include "Components/BoxComponent.h"


// Sets default values
AShipPartSnapCollider::AShipPartSnapCollider()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BottomCenter = CreateDefaultSubobject<USceneComponent>(TEXT("BottomCenterComp"));
	SetRootComponent(BottomCenter);

	FocusPoint = CreateDefaultSubobject<USceneComponent>(TEXT("FocusPointComp"));
	FocusPoint->SetupAttachment(GetRootComponent());

	SnapCollider = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SnapColliderComp"));
	SnapCollider->SetCollisionProfileName("ShipPartSnap");
	SnapCollider->SetupAttachment(GetRootComponent());
	SnapCollider->SetHiddenInGame(true);
}

// Called when the game starts or when spawned
void AShipPartSnapCollider::BeginPlay()
{
	Super::BeginPlay();
}
