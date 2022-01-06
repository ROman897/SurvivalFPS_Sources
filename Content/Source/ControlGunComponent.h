// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ToolComponent.h"
#include "ControlGunComponent.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API UControlGunComponent : public UToolComponent 
{
	GENERATED_BODY()

protected:
	bool bAutoGatheringResource = false;

	UPROPERTY(EditAnywhere)
	float TakeResourceTick;

	float CurrentTakeResourceTick;

	UPROPERTY(EditAnywhere)
	float GatherResourceRange;

public:
	USceneComponent* MuzzleLocation;

public:
	UControlGunComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void FirePressed() override;
	virtual void FireReleased() override;
	virtual bool CanFire() override;
	virtual bool NeedReload() override;
	virtual bool CanReload() override;
	virtual void Reload() override;
	virtual bool MustSendReleaseRPC() override;
};
