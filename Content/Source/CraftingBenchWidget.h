// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "InventoryWidget.h"

#include "CraftingBenchWidget.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API UCraftingBenchWidget : public UInventoryWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly)
	class ACraftingBench* LinkedCraftingBench;

public:
	void SetLinkedCraftingBench(class ACraftingBench* CraftingBench);
};
