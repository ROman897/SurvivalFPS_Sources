// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipFloatingMovement.h"

#include "Ship.h"


// Sets default values for this component's properties
UShipFloatingMovement::UShipFloatingMovement()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


float UShipFloatingMovement::GetRelativeSpeed()
{
	return Velocity.Size() / GetMaxSpeed();
}

void UShipFloatingMovement::CalculateEngineStats(float TotalHorsePower, float TotalWeight)
{
	MaxSpeed = 10 * TotalHorsePower / TotalWeight;
	Acceleration = TotalHorsePower / TotalWeight;
	Deceleration = Acceleration;
}

// Called when the game starts
void UShipFloatingMovement::BeginPlay()
{
	Super::BeginPlay();
	OwningShip = GetOwner<AShip>();
	check(OwningShip != nullptr);
}

bool UShipFloatingMovement::ResolvePenetrationImpl(const FVector& Adjustment, const FHitResult& Hit,
	const FQuat& NewRotationQuat)
{
	bPositionCorrected |= Super::ResolvePenetrationImpl(Adjustment, Hit, NewRotationQuat);
	return bPositionCorrected;
}

void UShipFloatingMovement::ApplyVelocityInput(float DeltaTime)
{
	float CurrentSpeed = Velocity.Size();
	
	const float ThrottleInput = GetPendingInputVector().X;
	UpdateThrottleValue(DeltaTime, ThrottleInput);

	float AvailableThrottle = OwningShip->ConsumeEnginePower(CurrentThrottleAmount, DeltaTime);
	const float TargetSpeed = GetMaxSpeed() * AvailableThrottle;

	const bool bExceedingTargetSpeed = CurrentSpeed > TargetSpeed;

	const float SpeedChange = (bExceedingTargetSpeed ? Deceleration : Acceleration) * DeltaTime;
	float NewSpeed;
	if (bExceedingTargetSpeed)
	{
		NewSpeed = FMath::Max(TargetSpeed, CurrentSpeed - SpeedChange);
	} else
	{
		NewSpeed = FMath::Min(TargetSpeed, CurrentSpeed + SpeedChange);
	}

	UE_LOG(LogTemp, Warning, TEXT("target speed: %f"), TargetSpeed);
	UE_LOG(LogTemp, Warning, TEXT("current speed: %f"), NewSpeed);

	Velocity = UpdatedComponent->GetForwardVector() * NewSpeed;
}

void UShipFloatingMovement::ApplyZRotationInput(float DeltaTime)
{
	const float ZRotationThrottleInput = GetPendingInputVector().Z;
	UpdateZRotationThrottleValue(DeltaTime, ZRotationThrottleInput);
	
	float SpeedMultiplier = FMath::Min(1.0f, Velocity.Size() / RequiredSpeedForMaxTurn);

	float TargetZAngularVelocity = MaxZAxisRotationSpeed * ZRotationThrottle * SpeedMultiplier;
	
	float CurrentZRotationSpeed = AngularVelocity.Yaw;

	bool bExceedingZAngularVelocity = CurrentZRotationSpeed > TargetZAngularVelocity;

	float FinalZAngularVelocity;

	if (bExceedingZAngularVelocity)
	{
		FinalZAngularVelocity = FMath::Max(TargetZAngularVelocity, CurrentZRotationSpeed - ZAxisRotationAcceleration * DeltaTime * SpeedMultiplier);
		
	} else
	{
		FinalZAngularVelocity = FMath::Min(TargetZAngularVelocity, CurrentZRotationSpeed + ZAxisRotationAcceleration * DeltaTime * SpeedMultiplier);
	}

	AngularVelocity.Yaw = FinalZAngularVelocity;
}

void UShipFloatingMovement::ApplyYRotationInput(float DeltaTime)
{
	const float YRotationThrottleInput = GetPendingInputVector().Y;
	UpdateYRotationThrottleValue(DeltaTime, YRotationThrottleInput);
	
	float SpeedMultiplier = FMath::Min(1.0f, Velocity.Size() / RequiredSpeedForMaxTurn);

	float TargetYAngularVelocity = MaxYAxisRotationSpeed * YRotationThrottle * SpeedMultiplier;
	
	float CurrentYRotationSpeed = AngularVelocity.Pitch;

	bool bExceedingYAngularVelocity = CurrentYRotationSpeed > TargetYAngularVelocity;

	float FinalYAngularVelocity;

	if (bExceedingYAngularVelocity)
	{
		FinalYAngularVelocity = FMath::Max(TargetYAngularVelocity, CurrentYRotationSpeed - YAxisRotationAcceleration * DeltaTime * SpeedMultiplier);
		
	} else
	{
		FinalYAngularVelocity = FMath::Min(TargetYAngularVelocity, CurrentYRotationSpeed + YAxisRotationAcceleration * DeltaTime * SpeedMultiplier);
	}

	AngularVelocity.Pitch = FinalYAngularVelocity;
}

void UShipFloatingMovement::UpdateThrottleValue(float DeltaTime, float ThrottleInput)
{
	if (ThrottleInput == 0.0f)
	{
		return;
	}

	CurrentThrottleAmount = FMath::Clamp(CurrentThrottleAmount + FMath::Sign(ThrottleInput) * DeltaTime * ThrottleChangeSpeed, 0.0f, 1.0f);
}

void UShipFloatingMovement::UpdateZRotationThrottleValue(float DeltaTime, float ThrottleInput)
{
	ZRotationThrottle = FMath::Clamp(ZRotationThrottle + FMath::Sign(ThrottleInput) * DeltaTime * ZRotationThrottleChangeSpeed, -1.0f, 1.0f);
}

void UShipFloatingMovement::UpdateYRotationThrottleValue(float DeltaTime, float ThrottleInput)
{
	YRotationThrottle = FMath::Clamp(YRotationThrottle + FMath::Sign(ThrottleInput) * DeltaTime * YRotationThrottleChangeSpeed, -1.0f, 1.0f);
}


// Called every frame
void UShipFloatingMovement::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	FrameCounter++;
	if (ShouldSkipUpdate(DeltaTime))
	{
		return;
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!PawnOwner || !UpdatedComponent)
	{
		return;
	}

	ApplyVelocityInput(DeltaTime);
	ApplyZRotationInput(DeltaTime);
	ApplyYRotationInput(DeltaTime);

	// TODO(Roman): only continue if this is the server
	// TODO(Roman): maybe differentiate between local player controller and AI controller

	bPositionCorrected = false;

	FVector MovementDelta = Velocity * DeltaTime;

	const FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FRotator OldRotation = UpdatedComponent->GetComponentRotation();

	FRotator DesiredRotation = UpdatedComponent->GetComponentRotation() + AngularVelocity * DeltaTime;
	DesiredRotation.Roll = AngularVelocity.Yaw / MaxZAxisRotationSpeed * RollForMaxZRotation;

	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(MovementDelta, DesiredRotation, true, Hit);

	if (Hit.IsValidBlockingHit())
	{
		HandleImpact(Hit, DeltaTime, MovementDelta);
		// Try to slide the remaining distance along the surface.
		SlideAlongSurface(MovementDelta, 1.f-Hit.Time, Hit.Normal, Hit, true);
	}

	// Update velocity
	// We don't want position changes to vastly reverse our direction (which can happen due to penetration fixups etc)
	if (!bPositionCorrected)
	{
		const FVector NewLocation = UpdatedComponent->GetComponentLocation();
		Velocity = ((NewLocation - OldLocation) / DeltaTime);

		const FRotator NewRotation = UpdatedComponent->GetComponentRotation();
		//AngularVelocity = (NewRotation - OldRotation) * (1 / DeltaTime);
	}

	ConsumeInputVector();

	UpdateComponentVelocity();
}

