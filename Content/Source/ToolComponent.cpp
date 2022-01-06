// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolComponent.h"
#include "BasicCharacter.h"

// Sets default values for this component's properties
UToolComponent::UToolComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UToolComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

FHitResult UToolComponent::LineTrace(const FVector& StartTrace, const FVector& EndTrace, ECollisionChannel CollisionChannel, bool DrawTrace) const
{
	if (DrawTrace)
	{
		GetWorld()->DebugDrawTraceTag = SCENE_QUERY_STAT(CollisionChannel);
	} else
	{
		GetWorld()->DebugDrawTraceTag = "";
	}
	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(CollisionChannel), true, GetOwner());
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, CollisionChannel, TraceParams);

	return Hit;
}

// Called every frame
void UToolComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UToolComponent::FirePressed()
{
}

void UToolComponent::FireReleased()
{
}

bool UToolComponent::CanFire()
{
	return false;
}

bool UToolComponent::NeedReload()
{
	return false;
}

bool UToolComponent::CanReload()
{
	return false;
}

void UToolComponent::Reload()
{
}

bool UToolComponent::MustSendReleaseRPC()
{
	return false;
}

void UToolComponent::AnimNotify(int NotifyNumber)
{
}

void UToolComponent::SetOwner(ABasicCharacter* OwnerChar)
{
	OwnerCharacter = OwnerChar;
}

EToolPose UToolComponent::GetToolPose()
{
	return ToolPose;
}

FWeaponAnimationData UToolComponent::GetWeaponAnimationData()
{
	return WeaponAnimationData;
}

void UToolComponent::SetMesh(UMeshComponent* MeshComponent)
{
	WeaponAnimationData.WeaponMesh = MeshComponent;
}

