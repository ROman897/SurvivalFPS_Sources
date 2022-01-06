// Fill out your copyright notice in the Description page of Project Settings.


#include "ReloadedMagazineNotify.h"
#include "BasicCharacter.h"

#include <Components/SkeletalMeshComponent.h>
#include <Animation/AnimSequenceBase.h>

void UReloadedMagazineNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	ABasicCharacter* OwnerBasicCharacter = Cast<ABasicCharacter>(MeshComp->GetOwner());
	check(OwnerBasicCharacter);
	OwnerBasicCharacter->OnNotifyToolReloaded();
}
