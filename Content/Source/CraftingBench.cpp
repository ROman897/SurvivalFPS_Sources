// Fill out your copyright notice in the Description page of Project Settings.


#include "CraftingBench.h"

#include "SharedNetworkedInventory.h"
#include "CraftingBenchWidget.h"
#include "NetworkedInventoryBase.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACraftingBench::ACraftingBench()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MainStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainStaticMeshComp"));
	SetRootComponent(MainStaticMesh);
}

// Called when the game starts or when spawned
void ACraftingBench::BeginPlay()
{
	Super::BeginPlay();
	
	FName WidgetName = FName(GetName() + FString("Widget"));
	check(CraftingBenchWidgetClass != nullptr);
	CraftingBenchWidget = CreateWidget<UCraftingBenchWidget>(UGameplayStatics::GetPlayerController(GetWorld(), 0), CraftingBenchWidgetClass, WidgetName);
	check(CraftingBenchWidget != nullptr);

	CraftingBenchWidget->SetLinkedCraftingBench(this);
	UNetworkedInventoryBase* PlayerBagInventory = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0))->GetNetworkedBagInventory();
	check(PlayerBagInventory != nullptr);
	CraftingBenchWidget->SetLinkedNetworkedInventories({PlayerBagInventory});
}

// Called every frame
void ACraftingBench::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bCraftingInProgress)
	{
		return;
	}
	check(SelectedRecipe != nullptr);
	
	CurrentCraftTick += DeltaTime;
	if (CurrentCraftTick >= RecipeCraftTime)
	{
		bool CanTakeRequiredResources = InteractingPlayerBagInventory->TryTakeRequiredResources(SelectedRecipe->RequiredResources);
		check(CanTakeRequiredResources);

		FItemInstance CraftedItemInstance {SelectedRecipe};
		int NumberOfCraftedItemsAdded = InteractingPlayerBagInventory->AddItem(CraftedItemInstance, 1);
		check(NumberOfCraftedItemsAdded == 1);
		

		if (CanCraftItem(SelectedRecipe, InteractingPlayerBagInventory))
		{
			CurrentCraftTick -= RecipeCraftTime;
		} else
		{
			CurrentCraftTick = 0.0f;
			CraftButtonReleased();
		}
	}
}

void ACraftingBench::Interact(APlayerCharacter* InteractingPlayer)
{
	InteractingPlayerBagInventory = InteractingPlayer->GetNetworkedBagInventory();
	check(InteractingPlayerBagInventory != nullptr);
	
	OpenPlayerWidget(InteractingPlayer, CraftingBenchWidget);
}

void ACraftingBench::SetIsFocused(bool IsFocused)
{
	SetMeshFocused(MainStaticMesh, IsFocused);
}

void ACraftingBench::SetSelectedRecipe(int32 RecipeIndex)
{
	check(!bCraftingInProgress);
	check(RecipeIndex >= 0);
	check(RecipeIndex < Recipes.Num());

	SelectedRecipeIndex = RecipeIndex;
	SelectedRecipe = Recipes[RecipeIndex];
	CurrentCraftTick = SelectedRecipe == PreviousRecipe ? PreviousCraftTick : 0.0f;
}

void ACraftingBench::CraftButtonPressed()
{
	check(InteractingPlayerBagInventory != nullptr);
	check(!bCraftingInProgress);
	check(SelectedRecipe != nullptr);

	if (!CanCraftItem(SelectedRecipe, InteractingPlayerBagInventory))
	{
		return;
	}
	
	if (SelectedRecipe != PreviousRecipe)
	{
		CurrentCraftTick = 0.0f;
		PreviousCraftTick = 0.0f;
	}
	PreviousRecipe = SelectedRecipe;
	RecipeCraftTime = SelectedRecipe->CraftTime;

	bCraftingInProgress = true;
}

void ACraftingBench::CraftButtonReleased()
{
	bCraftingInProgress = false;
	PreviousCraftTick = CurrentCraftTick;
}

bool ACraftingBench::CanCraftItem(UComposedItemDataAsset* ComposedItemDataAsset, UNetworkedInventoryBase* Inventory)
{
	check(Inventory != nullptr);
	check(ComposedItemDataAsset != nullptr);
	return Inventory->CanTakeRequiredResources(ComposedItemDataAsset->RequiredResources) && Inventory->CanAddItem(FItemInstance {SelectedRecipe}, 1);
}
