// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAnimNode_OffHandWeaponGrip.h"

#include "HumanoidAnimInstanceProxy.h"
#include "Animation/AnimInstanceProxy.h"
#include "SurvivalFPS/HumanoidAnimInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogWeaponPose, Log, All);

void FMyAnimNode_OffHandWeaponGrip::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	FAnimNode_SkeletalControlBase::Initialize_AnyThread(Context);

	bValidProxy = Context.AnimInstanceProxy->GetAnimInstanceObject() != nullptr && Context.AnimInstanceProxy->GetAnimInstanceObject()->IsA(UHumanoidAnimInstance::StaticClass());
	if (!bValidProxy)
	{
		UE_LOG(LogWeaponPose, Error, TEXT("Animation Instance must inherit from UMIAnimInstance. Unable to adjust left hand transform."));
	}
}

void FMyAnimNode_OffHandWeaponGrip::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	check(OutBoneTransforms.Num() == 0);

	if (!bValidProxy)
	{
		return;
	}

	// Cache owner
	USkeletalMeshComponent* const Mesh = Output.AnimInstanceProxy->GetSkelMeshComponent();
	if (!Mesh)
	{
		return;
	}

	// Move Offhand
	if (Weapon && Weapon.WeaponMesh->DoesSocketExist(Weapon.OffHandSocketName))
	{
		FTransform OffHandTM = Output.Pose.GetComponentSpaceTransform(OffHandPoseIndex);
		const FQuat HandRotation = OffHandTM.GetRotation();
		FTransform SocketWorldTM = Weapon.WeaponMesh->GetSocketTransform(Weapon.OffHandSocketName);

		OffHandTM = OffHandTM.GetRelativeTransform(SocketWorldTM);

		FVector BonePos;
		FRotator BoneRot;
		Mesh->TransformToBoneSpace(WeaponHandBone.BoneName, SocketWorldTM.GetLocation(), SocketWorldTM.Rotator(), BonePos, BoneRot);

		OffHandTM = FTransform::Identity;
		OffHandTM.SetLocation(BonePos);
		OffHandTM.SetRotation(BoneRot.Quaternion());

		FHumanoidAnimInstanceProxy* const AnimProxy = ((FHumanoidAnimInstanceProxy*)Output.AnimInstanceProxy);
		AnimProxy->OffHandIKTM = OffHandTM;
	}
}

bool FMyAnimNode_OffHandWeaponGrip::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{
	return WeaponHandBone.IsValidToEvaluate(RequiredBones) && OffHandIKBone.IsValidToEvaluate(RequiredBones);
}

void FMyAnimNode_OffHandWeaponGrip::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
	WeaponHandBone.Initialize(RequiredBones);
	OffHandIKBone.Initialize(RequiredBones);
	OffHandPoseIndex = OffHandIKBone.GetCompactPoseIndex(RequiredBones);
}
