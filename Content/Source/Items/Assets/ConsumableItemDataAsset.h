// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ComposedItemDataAsset.h"
#include "ConsumableItemDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FItemEffectsRanges
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HealthChange = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ArmorChange = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AttackPowerBoost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DefenseBoost = 0.0f;
};


/**
 * Class for Consumable items. Stores effects that should be applied.
 * TODO maybe extract potion subclass and this would be only for food
 */
UCLASS(BlueprintType)
class SURVIVALFPS_API UConsumableItemDataAsset : public UComposedItemDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FItemEffectsRanges ItemEffectsRange;
};
