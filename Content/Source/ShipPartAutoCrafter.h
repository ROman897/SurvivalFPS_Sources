// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "InteractableInterface.h"
#include "ShipPart.h"
#include "ShipPartAutoCrafter.generated.h"

UCLASS()
class SURVIVALFPS_API AShipPartAutoCrafter : public AShipPart, public IInteractableInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere)
	class USharedNetworkedInventory* InputInventory;

	UPROPERTY(VisibleAnywhere)
	class USharedNetworkedInventory* OutputInventory;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UShipPartAutoCrafterWidget> ShipPartAutoCrafterWidgetClass;

	UPROPERTY()
	class UShipPartAutoCrafterWidget* ShipPartAutoCrafterWidget;

	bool bCraftingInProgress = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<class UComposedItemDataAsset*> CraftableItems;

	UPROPERTY(BlueprintReadOnly)
	int CraftedComposedItemIndex = -1;

	class UComposedItemDataAsset* CraftedComposedItem;

	float CraftedComposedItemCraftTime;

	float CrafterTickTime;

	TArray<UPrimitiveComponent*> FocusablePrimitives;

	UPROPERTY(BlueprintReadOnly)
	float CraftingPercentageFinished = 0.0f;

public:
	// Sets default values for this actor's properties
	AShipPartAutoCrafter();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SetCraftedComposedItemIndex(int Index);

	void SetIsFocused(bool IsFocused) override;
	void Interact(APlayerCharacter* InteractingPlayer) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void TryStartCrafting();
	void StopCrafting();
	void SendInventoryItemsToPlayerAndClear();

	UFUNCTION()
	void InputInventoryChanged(const TArray<struct FItemInstanceStack>& NewInventoryItems);
	UFUNCTION()
	void OutputInventoryChanged(const TArray<struct FItemInstanceStack>& NewInventoryItems);
};
