// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AbilityAnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API UAbilityAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	FString AbilityName;

	UPROPERTY(EditAnywhere)
	int AbilityNotifyIndex;

protected:
	virtual void Notify(class USkeletalMeshComponent* MeshComp, class UAnimSequenceBase* Animation) override;
};
