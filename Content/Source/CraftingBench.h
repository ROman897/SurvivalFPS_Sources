// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "InteractableInterface.h"
#include "ShipPart.h"
#include "GameFramework/Actor.h"
#include "CraftingBench.generated.h"

UCLASS()
class SURVIVALFPS_API ACraftingBench : public AShipPart, public IInteractableInterface
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* MainStaticMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<class UComposedItemDataAsset*> Recipes;

	UPROPERTY(BlueprintReadOnly)
	int32 SelectedRecipeIndex = -1;

	bool bCraftingInProgress = false;

	class UComposedItemDataAsset* SelectedRecipe = nullptr;
	class UComposedItemDataAsset* PreviousRecipe = nullptr;

	UPROPERTY(BlueprintReadOnly)
	float CurrentCraftTick;

	float PreviousCraftTick = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float RecipeCraftTime;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UCraftingBenchWidget> CraftingBenchWidgetClass;

	UPROPERTY()
	class UCraftingBenchWidget* CraftingBenchWidget;

	class UNetworkedInventoryBase* InteractingPlayerBagInventory;

	float ShipTotalWeight = 0.0f;

public:
	// Sets default values for this actor's properties
	ACraftingBench();
	virtual void Tick(float DeltaTime) override;

	virtual void Interact(APlayerCharacter* InteractingPlayer) override;
	virtual void SetIsFocused(bool IsFocused) override;

	UFUNCTION(BlueprintCallable)
	void SetSelectedRecipe(int32 RecipeIndex);

	UFUNCTION(BlueprintCallable)
	void CraftButtonPressed();

	UFUNCTION(BlueprintCallable)
	void CraftButtonReleased();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool CanCraftItem(class UComposedItemDataAsset* ComposedItemDataAsset, class UNetworkedInventoryBase* Inventory);

};
