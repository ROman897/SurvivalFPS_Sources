// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToolComponent.h"
#include "AnimNotifiable.h"
#include <Containers/Set.h>

#include "MeeleResourceToolComponent.generated.h"

/**
 *
 */
UCLASS()
class SURVIVALFPS_API UMeeleResourceToolComponent : public UToolComponent
{
	GENERATED_BODY()

protected:
	bool bAutoFiring = false;
	float NextActionCooldown = 0.0f;
	bool bTriggerCanHit = false;

	UPROPERTY(EditAnywhere)
	float ActionCooldown;

	UPROPERTY(EditAnywhere)
	TSet<class UItemDataAsset*> CompatibleResources;

	UPROPERTY(EditAnywhere)
	int ResourceTakeOnHit;

public:
	UMeeleResourceToolComponent();

	virtual void FirePressed() override;
	virtual void FireReleased() override;
	virtual bool CanFire() override;
	virtual bool MustSendReleaseRPC() override;

	virtual void AnimNotify(int NofityNumber) override;

	UFUNCTION(BlueprintCallable)
	void OnHit(AActor* OtherActor);

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void TryAction();
};
