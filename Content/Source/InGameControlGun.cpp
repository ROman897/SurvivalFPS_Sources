// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameControlGun.h"

#include "ControlGunComponent.h"


// Sets default values
AInGameControlGun::AInGameControlGun()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainMeshComp"));
	SetRootComponent(MainMesh);

	MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocationComp"));
	MuzzleLocation->SetupAttachment(GetRootComponent());
	
	ControlGunComponent = CreateDefaultSubobject<UControlGunComponent>(TEXT("ControlGunComp"));
	ControlGunComponent->MuzzleLocation = MuzzleLocation;

	ToolComponent = ControlGunComponent;
}

// Called when the game starts or when spawned
void AInGameControlGun::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInGameControlGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

