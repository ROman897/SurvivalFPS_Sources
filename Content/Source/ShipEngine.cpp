// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipEngine.h"

// Sets default values
AShipEngine::AShipEngine()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MainBodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainBodyMeshComp"));
	MainBodyMesh->SetupAttachment(GetRootComponent());

	PropellerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PropellerMeshComp"));
	PropellerMesh->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AShipEngine::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AShipEngine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CurrentRelativePropellerRotationSpeed = FMath::FInterpConstantTo(CurrentRelativePropellerRotationSpeed, ShipThrottleValue, DeltaTime, PropellerRotationSpeedChange);
	PropellerMesh->AddRelativeRotation(FRotator(0, 0, CurrentRelativePropellerRotationSpeed * DeltaTime * PropellerMaxRotationSpeed));
}

float AShipEngine::GetHorsePower()
{
	return HorsePower;
}

float AShipEngine::GetPowerConsumptionMaxThrottlePerSecond()
{
	return PowerConsumptionMaxThrottlePerSecond;
}

void AShipEngine::SetShipThrottleValue(float ThrottleValue)
{
	ShipThrottleValue = ThrottleValue;
}

