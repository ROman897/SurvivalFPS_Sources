// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ReloadedMagazineNotify.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API UReloadedMagazineNotify : public UAnimNotify
{
	GENERATED_BODY()

	virtual void Notify(class USkeletalMeshComponent* MeshComp, class UAnimSequenceBase* Animation) override;
};
