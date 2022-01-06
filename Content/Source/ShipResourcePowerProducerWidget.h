// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "InventoryWidget.h"
#include "UObject/Object.h"
#include "ShipResourcePowerProducerWidget.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API UShipResourcePowerProducerWidget : public UInventoryWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly)
	class AShipResourcePowerProducer* LinkedShipResourcePowerProducer;

public:
	void SetLinkedShipResourcePowerProducer(class AShipResourcePowerProducer* ShipResourcePowerProducer)
	{
		LinkedShipResourcePowerProducer = ShipResourcePowerProducer;
	}
	
};
