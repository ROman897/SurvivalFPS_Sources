// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponAnimationData.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct SURVIVALFPS_API FWeaponAnimationData 
{
	GENERATED_BODY()

	FWeaponAnimationData()
		: WeaponMesh(nullptr)
		, OffHandSocketName("offhand_grip")
		, bIsOneHanded(false)
		, WeaponPose(nullptr)
		, AimOffset(nullptr)
	{}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	UMeshComponent* WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	FName OffHandSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	bool bIsOneHanded;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	UAnimSequenceBase* WeaponPose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	UBlendSpaceBase* AimOffset;

	FORCEINLINE bool operator==(const FWeaponAnimationData& Other) const
	{
		// First check is not default struct
		return !IsIdentical(FWeaponAnimationData()) && IsIdentical(Other);
	}

	FORCEINLINE bool operator!=(const FWeaponAnimationData& Other) const
	{
		return !(*this == Other);
	}

	FORCEINLINE bool IsIdentical(const FWeaponAnimationData& Other) const
	{
		return WeaponMesh == Other.WeaponMesh && OffHandSocketName.IsEqual(Other.OffHandSocketName) && bIsOneHanded == Other.bIsOneHanded && WeaponPose == Other.WeaponPose && AimOffset == Other.AimOffset;
	}

	FORCEINLINE operator bool() const
	{
		return IsValid();
	}

	FORCEINLINE bool IsValid() const
	{
		return WeaponMesh != nullptr;
	}
};
