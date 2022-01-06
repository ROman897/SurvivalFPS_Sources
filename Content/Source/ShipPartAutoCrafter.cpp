// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPartAutoCrafter.h"

#include "InGamePlayerController.h"
#include "PlayerCharacter.h"
#include "ShipPartAutoCrafterWidget.h"
#include "Kismet/GameplayStatics.h"
#include "SharedNetworkedInventory.h"


// Sets default values
AShipPartAutoCrafter::AShipPartAutoCrafter()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	InputInventory = CreateDefaultSubobject<USharedNetworkedInventory>(TEXT("InputInventoryComp"));
	OutputInventory = CreateDefaultSubobject<USharedNetworkedInventory>(TEXT("OutputInventoryComp"));

	InputInventory->InventoryChangedDelegate.AddDynamic(this, &AShipPartAutoCrafter::InputInventoryChanged);
	OutputInventory->InventoryChangedDelegate.AddDynamic(this, &AShipPartAutoCrafter::OutputInventoryChanged);
}

// Called when the game starts or when spawned
void AShipPartAutoCrafter::BeginPlay()
{
	Super::BeginPlay();

	check(InputInventory);
	check(OutputInventory);

	FName WidgetName = FName(GetName() + FString("Widget"));
	check(ShipPartAutoCrafterWidgetClass != nullptr);
	ShipPartAutoCrafterWidget = CreateWidget<UShipPartAutoCrafterWidget>(UGameplayStatics::GetPlayerController(GetWorld(), 0), ShipPartAutoCrafterWidgetClass, WidgetName);
	check(ShipPartAutoCrafterWidget != nullptr);
	ShipPartAutoCrafterWidget->SetLinkedShipPartAutoCrafter(this);
	
	UNetworkedInventoryBase* PlayerBagInventory = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0))->GetNetworkedBagInventory();
	ShipPartAutoCrafterWidget->SetLinkedNetworkedInventories({InputInventory, OutputInventory, PlayerBagInventory});
}

void AShipPartAutoCrafter::InputInventoryChanged(const TArray<FItemInstanceStack>& NewInventoryItems)
{
	if (CraftedComposedItem == nullptr)
	{
		return;
	}
	if (bCraftingInProgress)
	{
		if (!InputInventory->CanTakeRequiredResources(CraftedComposedItem->RequiredResources))
		{
			StopCrafting();
		}
	} else
	{
		TryStartCrafting();
	}
}

void AShipPartAutoCrafter::OutputInventoryChanged(const TArray<FItemInstanceStack>& NewInventoryItems)
{
	TryStartCrafting();
}

// Called every frame
void AShipPartAutoCrafter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bCraftingInProgress)
	{
		CraftingPercentageFinished = 0.0f;
		return;
	}
	check(CraftedComposedItem != nullptr);
	
	CrafterTickTime += DeltaTime;

	CraftingPercentageFinished = CrafterTickTime / CraftedComposedItemCraftTime;

	if (CrafterTickTime >= CraftedComposedItemCraftTime)
	{
		InputInventory->TakeRequiredResources(CraftedComposedItem->RequiredResources);
		int AddedCount = OutputInventory->AddItem(CraftedComposedItem, 1);
		check(AddedCount == 1);
		bCraftingInProgress = false;
		TryStartCrafting();
	}
}

void AShipPartAutoCrafter::SetCraftedComposedItemIndex(int Index)
{
	if (CraftedComposedItemIndex == Index)
	{
		return;
	}
	StopCrafting();
	CraftedComposedItemIndex = Index;
	if (Index == -1)
	{
		return;
	}
	CraftedComposedItem = CraftableItems[Index];
	SendInventoryItemsToPlayerAndClear();

	TArray<TSet<UItemDataAsset*>> AllowedItemDataAssets;
	AllowedItemDataAssets.SetNum(CraftedComposedItem->RequiredResources.Num());
	
	for (int i = 0; i < CraftedComposedItem->RequiredResources.Num(); ++i)
	{
		const FRequiredResource RequiredResource = CraftedComposedItem->RequiredResources[i];
		AllowedItemDataAssets[i].Add(RequiredResource.Item);
	}

	InputInventory->SetAllowedSlotItems(AllowedItemDataAssets);

	TryStartCrafting();
}

void AShipPartAutoCrafter::SetIsFocused(bool IsFocused)
{
	for (UPrimitiveComponent* FocusablePrimitive : FocusablePrimitives)
	{
		SetMeshFocused(FocusablePrimitive, IsFocused);
	}
}

void AShipPartAutoCrafter::Interact(APlayerCharacter* InteractingPlayer)
{
	AInGamePlayerController* PlayerController = InteractingPlayer->GetController<AInGamePlayerController>();
 	PlayerController->OpenInGameWidget(ShipPartAutoCrafterWidget);
}

void AShipPartAutoCrafter::TryStartCrafting()
{
	if (CraftedComposedItem == nullptr)
	{
		return;
	}
	
	if (bCraftingInProgress)
	{
		return;
	}
	if (InputInventory->CanTakeRequiredResources(CraftedComposedItem->RequiredResources)
		&& OutputInventory->CanAddItem(CraftedComposedItem, 1))
	{
		bCraftingInProgress = true;
		CrafterTickTime = 0.0f;
		CraftedComposedItemCraftTime = CraftedComposedItem->CraftTime;
	}
}

void AShipPartAutoCrafter::StopCrafting()
{
	bCraftingInProgress = false;
}

void AShipPartAutoCrafter::SendInventoryItemsToPlayerAndClear()
{
	// TODO(Roman)
}