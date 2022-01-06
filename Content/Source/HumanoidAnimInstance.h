// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Animation/WeaponAnimationData.h"
#include "UObject/Object.h"
#include "HumanoidAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API UHumanoidAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly)
	class ABasicCharacter* Character;

	UPROPERTY(BlueprintReadOnly)
	class UCharacterMovementComponent* MovementComponent;

	UPROPERTY(BlueprintReadOnly)
	class USkeletalMeshComponent* Mesh;

	UPROPERTY(BlueprintReadOnly)
	bool bIsApplyingInput = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsStrafeOrientedForward;

	/** (bIsApplyingInput & bIsMoving) - Used as "has started moving after stopping" */
	UPROPERTY(BlueprintReadOnly)
	bool bIsMovingWithInput = false;

	UPROPERTY(BlueprintReadOnly)
	FVector Velocity;

	UPROPERTY(BlueprintReadOnly)
	float Direction;

	UPROPERTY(BlueprintReadOnly)
	FVector RawInput;

	UPROPERTY(BlueprintReadOnly)
	float RawSpeed;

	UPROPERTY(BlueprintReadOnly)
	float NormalizedSpeed;

	UPROPERTY(BlueprintReadOnly)
	FVector StrafeDirection;
	
	/** Because Turn in Place prevents mesh from turning, need to rotate something to compensate by this amount (usually spine) */
	UPROPERTY(BlueprintReadWrite, Category = "AnimationState|Aim Offset")
	float AimOffsetTurn;

	/** Horizontal axis for aim offset */
	UPROPERTY(BlueprintReadWrite, Category = "AnimationState|Aim Offset")
	float AimOffsetYaw;

	/** Vertical axis for aim offset */
	UPROPERTY(BlueprintReadWrite, Category = "AnimationState|Aim Offset")
	float AimOffsetPitch;
	
	/** Transform applied using FABRIK to place offhand on weapon */
	UPROPERTY(BlueprintReadWrite, Category = "AnimationState|Weapon", meta = (DisplayName = "OffHand IK"))
	FTransform OffHandIKTM;

	UPROPERTY(BlueprintReadOnly)
	FWeaponAnimationData WeaponAnimationData;

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
	
	virtual void NativePostEvaluateAnimation() override;

protected:
	bool IsValidToEvaluate(float DeltaTime);
	void ComputeAimOffsets(float DeltaTime);
};

