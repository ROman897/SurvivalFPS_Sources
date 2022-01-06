// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipCharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "Engine/NetworkObjectList.h"
#include "GameFramework/Character.h"


// Sets default values for this component's properties
UShipCharacterMovementComponent::UShipCharacterMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	bImpartBaseVelocityX = false;
	bImpartBaseVelocityY = false;
	bImpartBaseVelocityZ = false;

	// ...
}

void UShipCharacterMovementComponent::UpdateBasedMovement(float DeltaTime)
{
	if (UpdatedComponent->GetAttachParent() != nullptr)
	{
		return;
	}
	Super::UpdateBasedMovement(DeltaTime);
}


FActorComponentTickFunction& UShipCharacterMovementComponent::GetPrimaryTickFunction()
{
	return PrimaryComponentTick;
}

// Called when the game starts
void UShipCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UShipCharacterMovementComponent::SetBase(UPrimitiveComponent* NewBase, const FName BoneName, bool bNotifyActor)
{
}

// Called every frame
void UShipCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                          FActorComponentTickFunction* ThisTickFunction)
{
	++FrameCounter;
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

