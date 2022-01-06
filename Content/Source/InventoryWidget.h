// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include <Containers/Array.h>
#include "InventoryWidget.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class SURVIVALFPS_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly)
	TArray<class UNetworkedInventoryBase*> LinkedNetworkedInventories;

public:
	void SetLinkedNetworkedInventories(const TArray<UNetworkedInventoryBase*>& NetworkedInventories);
};
