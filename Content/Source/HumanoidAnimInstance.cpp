// Fill out your copyright notice in the Description page of Project Settings.


#include "HumanoidAnimInstance.h"

#include "BasicCharacter.h"
#include "Animation/HumanoidAnimInstanceProxy.h"
#include "Gameframework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UHumanoidAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<ABasicCharacter>(TryGetPawnOwner());

	if (Character != nullptr)
	{
		MovementComponent = Character->GetCharacterMovement();
		check(MovementComponent != nullptr);

		Mesh = Character->GetMesh();
		check(Mesh != nullptr);
	}
}

void UHumanoidAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (!IsValidToEvaluate(DeltaTime))
	{
		return;
	}

	Velocity = Character->GetVelocity();
	
	RawSpeed = (MovementComponent->IsMovingOnGround()) ? Velocity.Size() : Velocity.Size2D();
	NormalizedSpeed = UKismetMathLibrary::NormalizeToRange(RawSpeed, 0.f, MovementComponent->GetMaxSpeed());

	// TODO(Roman): what about root motion
	bIsMoving = RawSpeed > 0.0f;

	RawInput = Character->GetLastRawMovementInput();

	bIsApplyingInput = !RawInput.IsNearlyZero();

	bIsStrafeOrientedForward = RawInput.X >= 0.0f;

	bIsMovingWithInput = bIsApplyingInput && bIsMoving;

	Direction = CalculateDirection(Velocity, Character->GetActorRotation());

	StrafeDirection = bIsStrafeOrientedForward ? RawInput : -RawInput;

	ComputeAimOffsets(DeltaTime);

	FWeaponAnimationData NewWeaponAnimationData = Character->GetWeaponAnimationData();
	WeaponAnimationData = NewWeaponAnimationData;
}

FAnimInstanceProxy* UHumanoidAnimInstance::CreateAnimInstanceProxy()
{
	return new FHumanoidAnimInstanceProxy(this);
}

void UHumanoidAnimInstance::NativePostEvaluateAnimation()
{
	// Doesn't actually have anything in super
	Super::NativePostEvaluateAnimation();

	FHumanoidAnimInstanceProxy& Proxy = GetProxyOnGameThread<FHumanoidAnimInstanceProxy>();
	OffHandIKTM = Proxy.OffHandIKTM;
}

bool UHumanoidAnimInstance::IsValidToEvaluate(float DeltaTime)
{
	if (!Character || !MovementComponent || !Mesh)
	{
		return false;
	}

	if (DeltaTime < 1e-6f)
	{
		return false;
	}

	return true;
}

void UHumanoidAnimInstance::ComputeAimOffsets(float DeltaTime)
{
	// this is basically rotation of the character
	const FRotator MeshRotation = (Mesh->GetComponentRotation() + FRotator(0.f, 90.f, 0.f)).GetNormalized();

	// this is the difference between where we're aiming and where the character is rotated
	const FRotator AimDelta = (Character->GetBaseAimRotation() - MeshRotation).GetNormalized();

	AimOffsetYaw = -AimDelta.Yaw;

	AimOffsetPitch = AimDelta.Pitch;
}
