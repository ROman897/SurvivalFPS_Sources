// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FloatingPickableItemDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API UFloatingPickableItemDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	class UItemDataAsset* ItemDataAsset;

	// how many instances are in 100 x 100 x 100 meters 
	UPROPERTY(EditAnywhere)
	float MinOccurencesPer100x100x100M;
	
	UPROPERTY(EditAnywhere)
	float MaxOccurencesPer100x100x100M;
};
