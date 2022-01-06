// Fill out your copyright notice in the Description page of Project Settings.


#include "InstancedEnvironmentPropComponent.h"

#include "EnvironmentPropDataAsset.h"
#include "NetworkedInventoryBase.h"


// Sets default values for this component's properties
UInstancedEnvironmentPropComponent::UInstancedEnvironmentPropComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
}


// Called when the game starts
void UInstancedEnvironmentPropComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UInstancedEnvironmentPropComponent::SetEnvironmentPropDataAsset(
	const UEnvironmentPropDataAsset* NewEnvironmentPropDataAsset)
{
	EnvironmentPropDataAsset = NewEnvironmentPropDataAsset;

	SetStaticMesh(EnvironmentPropDataAsset->PropStaticMesh);
	
	if (EnvironmentPropDataAsset->NoCollision)
	{
		SetCollisionProfileName("NoCollision");
	} else
	{
		SetCollisionProfileName("EnvironmentPropWorldStatic");
		SetGenerateOverlapEvents(true);
		bMultiBodyOverlap = true;
		bFillCollisionUnderneathForNavmesh = true;
	}
	SetCullDistances(EnvironmentPropDataAsset->CullStartDistance, EnvironmentPropDataAsset->CullCompletelyDistance);
	
	for (int i = 0; i < EnvironmentPropDataAsset->Materials.Num(); ++i)
	{
		SetMaterial(i, EnvironmentPropDataAsset->Materials[i]);
	}
}

