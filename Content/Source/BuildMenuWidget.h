// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include <Containers/Array.h>
#include "BuildMenuWidget.generated.h"

/**
 *
 */

USTRUCT(BlueprintType)
struct SURVIVALFPS_API FBuildingMenuCategory {
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<class UShipPartDataAsset*> Buildings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString CategoryName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UTexture2D* CategoryIcon;
};

UCLASS()
class SURVIVALFPS_API UBuildMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FBuildingMenuCategory> BuildingMenuCategories;

public:
	bool Toggle();
};
