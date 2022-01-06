// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "ShipCharacterMovementComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SURVIVALFPS_API UShipCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

protected:
	bool bLandedThisFrame = false;
	float LandedThisFrameGroundTime;

	int FrameCounter = 0;

public:
	// Sets default values for this component's properties
	UShipCharacterMovementComponent();

	void UpdateBasedMovement(float DeltaSeconds) override;
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	void SetVelocity(FVector NewVelocity)
	{
		Velocity = NewVelocity;
	}
	
	FActorComponentTickFunction& GetPrimaryTickFunction();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	/**
	 * Calculate slide vector along a surface, if surface is moving then don't cancel all movement speed.
	 * Has special treatment when falling, to avoid boosting up slopes (calling HandleSlopeBoosting() in this case).
	 *
	 * @param Delta:	Attempted move.
	 * @param Time:		Amount of move to apply (between 0 and 1).
	 * @param Normal:	Normal opposed to movement. Not necessarily equal to Hit.Normal (but usually is).
	 * @param Hit:		HitResult of the move that resulted in the slide.
	 * @return			New deflected vector of movement.
	 */
	virtual void SetBase(UPrimitiveComponent* NewBase, const FName BoneName = NAME_None, bool bNotifyActor = true) override;
};
