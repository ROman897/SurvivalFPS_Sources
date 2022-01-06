// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/PawnMovementComponent.h"

#include "ShipFloatingMovement.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SURVIVALFPS_API UShipFloatingMovement : public UPawnMovementComponent
{
	GENERATED_BODY()

protected:
	float MaxSpeed = 0.0f;

	float Acceleration = 0.0f;

	float Deceleration = 0.0f;
	
	UPROPERTY(EditAnywhere)
	float ThrottleChangeSpeed;
	
	// this is equivalent of car throttle
	float CurrentThrottleAmount = 0.0f;

	bool bPositionCorrected = false;

	UPROPERTY(EditAnywhere)
	float MaxZAxisRotationSpeed;

	UPROPERTY(EditAnywhere)
	float ZAxisRotationAcceleration;

	// ranges -1 to 1
	float ZRotationThrottle = 0.0f;

	UPROPERTY(EditAnywhere)
	float ZRotationThrottleChangeSpeed;

	UPROPERTY(EditAnywhere)
	float MaxYAxisRotationSpeed;

	UPROPERTY(EditAnywhere)
	float YAxisRotationAcceleration;

	// ranges -1 to 1
	float YRotationThrottle = 0.0f;
	
	UPROPERTY(EditAnywhere)
	float YRotationThrottleChangeSpeed;
	
	FRotator AngularVelocity = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere)
	float RollForMaxZRotation;

	UPROPERTY(EditAnywhere)
	float RequiredSpeedForMaxTurn;

	float MaxAvailableThrottle = 0.0f;

	class AShip* OwningShip;
	
	int FrameCounter = 0;

public:
	// Sets default values for this component's properties
	UShipFloatingMovement();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual float GetMaxSpeed() const override { return MaxSpeed; }

	float GetRelativeSpeed();
	void CalculateEngineStats(float TotalHorsePower, float TotalWeight);

	FRotator GetRelativeAngularVelocity()
	{
		return FRotator(AngularVelocity.Pitch / MaxYAxisRotationSpeed, AngularVelocity.Yaw / MaxZAxisRotationSpeed, 0.0f);
	}

	float GetThrottleAmount()
	{
		return CurrentThrottleAmount;
	}

	float GetZRotationThrottleAmount()
	{
		return ZRotationThrottle;
	}

	float GetYRotationThrottleAmount()
	{
		return YRotationThrottle;
	}
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual bool ResolvePenetrationImpl(const FVector& Adjustment, const FHitResult& Hit, const FQuat& NewRotationQuat) override;
	void ApplyVelocityInput(float DeltaTime);
	void ApplyZRotationInput(float DeltaTime);
	void ApplyYRotationInput(float DeltaTime);
	void UpdateThrottleValue(float DeltaTime, float ThrottleInput);
	void UpdateZRotationThrottleValue(float DeltaTime, float ThrottleInput);
	void UpdateYRotationThrottleValue(float DeltaTime, float ThrottleInput);
};

