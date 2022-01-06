// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemDataAsset.h"

#include "ComposedItemDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FRequiredResource
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UItemDataAsset* Item;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Count = 0;
};

USTRUCT(BlueprintType)
struct FYieldResource
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int MinCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int MaxCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UItemDataAsset* Item;
};

/**
 * Class for composed Items in inventory. Describe required resources and dismantle results.
 */
UCLASS(BlueprintType)
class SURVIVALFPS_API UComposedItemDataAsset : public UItemDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FRequiredResource> RequiredResources;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FYieldResource> YieldResources;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CraftTime;
};
