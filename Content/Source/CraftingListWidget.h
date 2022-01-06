// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include <Containers/Array.h>
#include "CraftingListWidget.generated.h"

/**
 *
 */
UCLASS()
class SURVIVALFPS_API UCraftingListWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<class UComposedItemDataAsset*> CraftableItems;

public:
	bool Toggle();

};
