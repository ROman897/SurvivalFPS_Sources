// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Components/SphereComponent.h"
#include "UObject/Object.h"
#include "PlayerPropsReplacer.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API UPlayerPropsReplacer : public USphereComponent
{
	GENERATED_BODY()

public:
	UPlayerPropsReplacer();
	
protected:
	UFUNCTION()
	void OnPropReplacementTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnPropActorReplacementTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
