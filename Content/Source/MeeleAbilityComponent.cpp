// Fill out your copyright notice in the Description page of Project Settings.


#include "MeeleAbilityComponent.h"
#include "BasicCharacter.h"
#include <Gameframework/CharacterMovementComponent.h>

void UMeeleAbilityComponent::Onhit(AActor* OtherActor) {
	if (bTriggerCanHit) {
		if (OtherActor == GetOwner()) {
			return;
		}
		ABasicCharacter* HitCharacter = Cast<ABasicCharacter>(OtherActor);
		check(HitCharacter != nullptr);
		HitCharacter->OnHit(Damage);
		UE_LOG(LogTemp, Warning, TEXT("adding force: %s"), *(GetOwner()->GetActorForwardVector() * 10000.0f).ToString());
		HitCharacter->LaunchCharacter((GetOwner()->GetActorForwardVector() + 0.6f * GetOwner()->GetActorUpVector()) * 500.0f, false, false);
	}
}

void UMeeleAbilityComponent::AnimNotify(int NotifyNumber)
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
