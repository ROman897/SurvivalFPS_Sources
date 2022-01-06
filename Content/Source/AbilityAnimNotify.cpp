// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityAnimNotify.h"
#include "BasicCharacter.h"
#include "AbilityComponent.h"

void UAbilityAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	ABasicCharacter* OwnerCharacter = MeshComp->GetOwner<ABasicCharacter>();
	if (OwnerCharacter != nullptr) {
		UAbilityComponent* Ability = OwnerCharacter->GetAbilityByName(AbilityName);
		if (Ability != nullptr) {
			Ability->AnimNotify(AbilityNotifyIndex);
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("AbilityAnimNotify OwnerCharacter nullptr"));
	}
}
