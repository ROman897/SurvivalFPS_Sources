// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ComposedItemDataAsset.h"

#include "EquipableItemDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FItemStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HealthBoost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ArmorBoost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AttackPowerBoost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DefenseBoost = 0.0f;
};

/**
 * Class for simple Equipable item. Stores Spawnable Class.
 */
UCLASS(BlueprintType)
class SURVIVALFPS_API UEquipableItemDataAsset : public UComposedItemDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FItemStats ItemEffectsRange;

	// linked in-game class of this item, i.e. gun that is held by a player
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> SpawnableClass;
};
