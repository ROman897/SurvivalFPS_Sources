// Fill out your copyright notice in the Description page of Project Settings.


#include "InstancedEnvironmentResourcePropComponent.h"

#include "EnvironmentPropDataAsset.h"
#include "InGameResource.h"
#include "InGameResourceDataAsset.h"
#include "NetworkedInventoryBase.h"
#include "PlayerCharacter.h"


// Sets default values for this component's properties
UInstancedEnvironmentResourcePropComponent::UInstancedEnvironmentResourcePropComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UInstancedEnvironmentResourcePropComponent::ReplacePropByActor(int32 InstanceId)
{
	check(EnvironmentPropDataAsset->InGameResourceDataAsset != nullptr);
	check(EnvironmentPropDataAsset->InGameResourceDataAsset->InGameResourceActorClass != nullptr);
	FTransform InstanceTransform;
	GetInstanceTransform(InstanceId, InstanceTransform, true);
	RemoveInstance(InstanceId);
	AInGameResource* InGameResource = GetWorld()->SpawnActor<AInGameResource>(EnvironmentPropDataAsset->InGameResourceDataAsset->InGameResourceActorClass, InstanceTransform);
	check(InGameResource != nullptr);

	FResourceState* ResourceState = InGameResourceStates.Find(InstanceId);
	check(ResourceState != nullptr);
	
	InGameResource->InitResource(EnvironmentPropDataAsset->InGameResourceDataAsset, ResourceState->HitsLeft, this, InstanceId);
}

void UInstancedEnvironmentResourcePropComponent::ReclaimPropActor(int32 InstanceId,
	int32 HitsLeft, const FTransform& InstanceTransform)
{
	check(HitsLeft > 0);
	check(InGameResourceStates.Contains(InstanceId));
	InGameResourceStates.Emplace(InstanceId, FResourceState{HitsLeft});
	AddInstance(InstanceTransform);
}

void UInstancedEnvironmentResourcePropComponent::PropActorDestroyed(int32 InstanceId)
{
	check(InGameResourceStates.Contains(InstanceId));
	InGameResourceStates.Remove(InstanceId);
}


// Called when the game starts
void UInstancedEnvironmentResourcePropComponent::BeginPlay()
{
	Super::BeginPlay();
}

int32 UInstancedEnvironmentResourcePropComponent::AddInstance(const FTransform& InstanceTransform)
{
	check(EnvironmentPropDataAsset->InGameResourceDataAsset != nullptr);
	
	int32 Id = Super::AddInstance(InstanceTransform);

	FResourceState ResourceState {
		EnvironmentPropDataAsset->InGameResourceDataAsset->MaxHits,
	};
	InGameResourceStates.Emplace(Id, MoveTemp(ResourceState));

	return Id;
}