// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipControls.h"

#include "Ship.h"
#include "ShipFloatingMovement.h"


// Sets default values
AShipControls::AShipControls()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MainMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainMeshComp"));
	MainMeshComponent->SetupAttachment(GetRootComponent());

	DirectionStickRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DirectionStickRootComp"));
	DirectionStickRootComponent->SetupAttachment(MainMeshComponent);
	
	ThrottleStickRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ThrottleStickRootComp"));
	ThrottleStickRootComponent->SetupAttachment(MainMeshComponent);
}

// Called when the game starts or when spawned
void AShipControls::BeginPlay()
{
	Super::BeginPlay();
	AShip* Ship = Cast<AShip>(GetParentActor());
	check(Ship != nullptr);
	SetShip(Ship);
}

// Called every frame
void AShipControls::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	check(OwningShip != nullptr);
	check(ShipFloatingMovement != nullptr);

	DirectionStickRootComponent->SetRelativeRotation(FRotator(-ShipFloatingMovement->GetYRotationThrottleAmount() * RotationStickMaxPitch, 0.0f, -ShipFloatingMovement->GetZRotationThrottleAmount() * RotationStickMaxYaw));

	ThrottleStickRootComponent->SetRelativeRotation(FRotator(0.0f, -ShipFloatingMovement->GetThrottleAmount() * ThrottleStickMaxRotation, 0.0f));
}

void AShipControls::SetIsFocused(bool IsFocused)
{
	SetMeshFocused(MainMeshComponent, IsFocused);
}

void AShipControls::Interact(APlayerCharacter* InteractingPlayer)
{
	InteractingPlayer->SetCurrentShipControls(this);
}

void AShipControls::PlayerControlCancelled()
{
}

void AShipControls::AddSteering(float Direction)
{
	OwningShip->AddSteering(Direction);
}

void AShipControls::AddSpeed(float Direction)
{
	OwningShip->AddThrottle(Direction);
	
}

void AShipControls::AddPitch(float Direction)
{
	OwningShip->AddPitch(Direction);
}

void AShipControls::SetShip(AShip* Ship)
{
	OwningShip = Ship;
	ShipFloatingMovement = OwningShip->GetShipFloatingMovement();
}
