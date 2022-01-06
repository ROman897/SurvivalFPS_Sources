// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "InventoryWidget.h"
#include "UObject/Object.h"
#include "ShipPartAutoCrafterWidget.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API UShipPartAutoCrafterWidget : public UInventoryWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly)
	class AShipPartAutoCrafter* LinkedShipPartAutoCrafter;

public:
	void SetLinkedShipPartAutoCrafter(class AShipPartAutoCrafter* ShipPartAutoCrafter)
	{
		LinkedShipPartAutoCrafter = ShipPartAutoCrafter;
	}
};
