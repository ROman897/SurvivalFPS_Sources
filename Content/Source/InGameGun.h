// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InGameTool.h"
#include "InGameGun.generated.h"

UCLASS()
class SURVIVALFPS_API AInGameGun : public AInGameTool
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere)
	class USkeletalMeshComponent* GunMesh;

	UPROPERTY(VisibleAnywhere)
	class USceneComponent* GunMuzzle;

	UPROPERTY(VisibleAnywhere)
	class UGunComponent* GunComponent;

public:	
	// Called every frame
	AInGameGun();
};
