// Fill out your copyright notice in the Description page of Project Settings.


#include "MeeleResourceToolComponent.h"
#include "BasicCharacter.h"
#include "InGameResource.h"
#include "PlayerCharacter.h"

UMeeleResourceToolComponent::UMeeleResourceToolComponent()
{
}

void UMeeleResourceToolComponent::FirePressed()
{
	bAutoFiring = true;
}

void UMeeleResourceToolComponent::FireReleased()
{
	bAutoFiring = false;
}

bool UMeeleResourceToolComponent::CanFire()
{
	return true;
}

bool UMeeleResourceToolComponent::MustSendReleaseRPC()
{
	return true;
}

void UMeeleResourceToolComponent::AnimNotify(int NotifyNumber)
{
	switch (NotifyNumber) {
		case 0:
			bTriggerCanHit = true;
			break;
		case 1:
			bTriggerCanHit = false;
			break;
		default:
			check(false);
	}
}

void UMeeleResourceToolComponent::OnHit(AActor* OtherActor)
{
	if (!bTriggerCanHit || OtherActor == Cast<AActor>(OwnerCharacter)) {
		return;
	}
	AInGameResource* InGameResource = Cast<AInGameResource>(OtherActor);
	if (InGameResource == nullptr) {
		return;
	}
	/*if (CompatibleResources.Contains(InGameResource->GetDroppedItemDataAsset())) {
		InGameResource->OnHit(Cast<APlayerCharacter>(OwnerCharacter)->GetNetworkedBagInventory(), ResourceTakeOnHit);
	}*/
}

void UMeeleResourceToolComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (NextActionCooldown > 0.0f) {
		NextActionCooldown -= DeltaTime;
	}
	TryAction();
}

void UMeeleResourceToolComponent::TryAction()
{
	if (!bAutoFiring || NextActionCooldown > 0.0f) {
		return;
	}
	// 
	NextActionCooldown = ActionCooldown;
	OwnerCharacter->ToolFired();
}
