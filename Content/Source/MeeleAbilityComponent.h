// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilityComponent.h"

#include "MeeleAbilityComponent.generated.h"

/**
 * 
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIVALFPS_API UMeeleAbilityComponent : public UAbilityComponent
{
	GENERATED_BODY()

private:
	bool bTriggerCanHit = false;

protected:
	UPROPERTY(EditAnywhere)
	float Damage;

public:
	UFUNCTION(BlueprintCallable)
	void Onhit(AActor* OtherActor);

	virtual void AnimNotify(int NotifyNumber) override;
};
