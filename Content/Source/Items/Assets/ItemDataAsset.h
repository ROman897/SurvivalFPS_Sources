// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDataAsset.generated.h"

/**
 * Class for simple Item that can be picked from the ground and stored in Inventory.
 */
UCLASS(BlueprintType)
class SURVIVALFPS_API UItemDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UTexture2D* ItemIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int MaxCountInStack;

	// pickable class of this item, i.e. gun lying on the floor
	UPROPERTY(EditAnywhere)
	TSubclassOf<class APickableItem> PickableItemClass;
};
