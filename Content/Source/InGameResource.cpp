// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameResource.h"

#include "InGameResourceDataAsset.h"
#include "NetworkedInventoryBase.h"
#include "InstancedEnvironmentResourcePropComponent.h"

// Sets default values
AInGameResource::AInGameResource()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainMeshComp"));
	SetRootComponent(MainMesh);
}

void AInGameResource::InitResource(UInGameResourceDataAsset* MyInGameResourceDataAsset,
	int MyHitsLeft,
	UInstancedEnvironmentResourcePropComponent* MyResourceInstancer,
	int32 MyInstanceId)
{
	InGameResourceDataAsset = MyInGameResourceDataAsset;
	HitsLeft = MyHitsLeft;
	ResourceInstancer = MyResourceInstancer;
	InstanceId = MyInstanceId;
}

void AInGameResource::OnHit(UNetworkedInventoryBase* TargetInventory, int NumberOfHits)
{
	int HitsToTake = FMath::Min(HitsLeft, NumberOfHits);
	if (HitsToTake <= 0)
	{
		check(false);
	}
	TargetInventory->AddItem(InGameResourceDataAsset->InGameResourceItemDataAsset, InGameResourceDataAsset->ItemDropPerHit * HitsToTake);
	HitsLeft -= HitsToTake;

	if (HitsLeft == 0)
	{
		ResourceInstancer->PropActorDestroyed(InstanceId);
		Destroy();
	}
}

void AInGameResource::GetReclaimed()
{
	if (IsPendingKillPending())
	{
		return;
	}
	check(HitsLeft > 0);
	ResourceInstancer->ReclaimPropActor(InstanceId, HitsLeft, GetTransform());
	Destroy();
}

void AInGameResource::Interact(APlayerCharacter* InteractingPlayer)
{
	OnHit(InteractingPlayer->GetNetworkedBagInventory(), 1);
}

void AInGameResource::SetIsFocused(bool IsFocused)
{
	SetMeshFocused(MainMesh, IsFocused);
}

bool AInGameResource::IsContinuousInteraction()
{
	return true;
}

float AInGameResource::GetContinuousInteractionCooldown()
{
	return GatherResourceCooldown;
}
