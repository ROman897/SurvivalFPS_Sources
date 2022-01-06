// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "InGameResourceDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API UInGameResourceDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AInGameResource> InGameResourceActorClass;
	
	UPROPERTY(EditAnywhere)
	class UItemDataAsset* InGameResourceItemDataAsset;

	// TODO(Roman): maybe make this a range/distribution?
	UPROPERTY(EditAnywhere)
	int32 MaxHits;

	UPROPERTY(EditAnywhere)
	int32 ItemDropPerHit;
};
