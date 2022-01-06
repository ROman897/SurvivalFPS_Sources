// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolAnimNotify.h"
#include "BasicCharacter.h"
#include "ToolComponent.h"

void UToolAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	ABasicCharacter* OwnerCharacter = MeshComp->GetOwner<ABasicCharacter>();
	if (OwnerCharacter != nullptr) {
		UToolComponent* PrimaryToolComponent = OwnerCharacter->GetPrimaryToolComponent();
		if (PrimaryToolComponent != nullptr) {
			PrimaryToolComponent->AnimNotify(NotifyIndex);
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Triggered tool notify but character has no tool"));
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("AbilityAnimNotify OwnerCharacter nullptr"));
	}
}
