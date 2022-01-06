// Fill out your copyright notice in the Description page of Project Settings.


#include "ControlGunComponent.h"

#include "CollisionTraceChannels.h"
#include "InGameResource.h"
#include "NetworkedInventoryBase.h"
#include "PlayerCharacter.h"

UControlGunComponent::UControlGunComponent()
{
}

void UControlGunComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	/*if (!bAutoGatheringResource)
	{
		return;
	}
	CurrentTakeResourceTick += DeltaTime;

	if (CurrentTakeResourceTick >= TakeResourceTick)
	{
		CurrentTakeResourceTick -= TakeResourceTick;
	} else
	{
		return;
	}

	check(MuzzleLocation != nullptr);
	FCollisionQueryParams CollisionQueryParams;
	FVector StartTrace = MuzzleLocation->GetComponentLocation();
	FVector EndTrace = StartTrace + MuzzleLocation->GetForwardVector() * GatherResourceRange;
	FHitResult HitResult = LineTrace(StartTrace, EndTrace, TRACE_CHANNEL_SOLID_MATTER);

	IResourceSourceInterface* ResourceSource = Cast<IResourceSourceInterface>(HitResult.Component);

	if (ResourceSource == nullptr)
	{
		return;
	}
	
	APlayerCharacter* OwnerPlayer = Cast<APlayerCharacter>(OwnerCharacter);
	check(OwnerPlayer != nullptr);
	UE_LOG(LogTemp, Warning, TEXT("resource gun hit %s"), *HitResult.Component->GetName());
	ResourceSource->Hit(1, HitResult.ElementIndex, OwnerPlayer->GetNetworkedBagInventory());
	
	/*AInGameResource* HitResource = Cast<AInGameResource>(HitResult.Actor);
	if (HitResource == nullptr)
	{
		if (HitResult.Actor != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("resource gun hit %s"), *HitResult.Actor->GetName());
		} else
		{
			UE_LOG(LogTemp, Warning, TEXT("resource gun hit nothin"));
		}
		return;
	}

	/*APlayerCharacter* OwnerPlayer = Cast<APlayerCharacter>(OwnerCharacter);
	check(OwnerPlayer != nullptr);
	UE_LOG(LogTemp, Warning, TEXT("resource gun hit %s"), *HitResource->GetName());
	HitResource->OnHit(OwnerPlayer->GetNetworkedBagInventory());
	*/
}

void UControlGunComponent::FirePressed()
{
	bAutoGatheringResource = true;
}

void UControlGunComponent::FireReleased()
{
	bAutoGatheringResource = false;
}

bool UControlGunComponent::CanFire()
{
	return true;
}

bool UControlGunComponent::NeedReload()
{
	return false;
}

bool UControlGunComponent::CanReload()
{
	return false;
}

void UControlGunComponent::Reload()
{
	check(false);
}

bool UControlGunComponent::MustSendReleaseRPC()
{
	return true;
}
