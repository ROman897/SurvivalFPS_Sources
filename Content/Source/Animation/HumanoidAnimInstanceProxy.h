// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstanceProxy.h"
#include "HumanoidAnimInstanceProxy.generated.h"


/**
 * Used to pass data in/out of threaded animation nodes
 * This is used specifically to get the correct position for the offhand placed on a weapon
 */
USTRUCT(meta = (DisplayName = "Native Variables"))
struct FHumanoidAnimInstanceProxy : public FAnimInstanceProxy
{
	GENERATED_BODY()

	FTransform OffHandIKTM;

	FHumanoidAnimInstanceProxy()
		: Super()
		, OffHandIKTM(FTransform::Identity)
	{}

	FHumanoidAnimInstanceProxy(UAnimInstance* Instance)
		: Super(Instance)
		, OffHandIKTM(FTransform::Identity)
	{}

	virtual ~FHumanoidAnimInstanceProxy() {}
};
