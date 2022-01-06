#include "NetworkedInventoryBase.h"
#include "InGamePlayerController.h"
#include "Items/ItemInstance.h"

#include <Net/UnrealNetwork.h>
#include <Blueprint/UserWidget.h>
#include <Blueprint/WidgetBlueprintLibrary.h>
#include <Kismet/GameplayStatics.h>

#include "Items/Assets/ItemDataAsset.h"

// Sets default values for this component's properties
UNetworkedInventoryBase::UNetworkedInventoryBase()
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);
}

void UNetworkedInventoryBase::ServerRPCOnItemDrop_Implementation(UNetworkedInventoryBase* FromInventory, int FromSlotId, int ToSlotId)
{
	if (FromInventory == nullptr) {
		// this is actually a valid scenario, if player tries to get an item from inventory that is begin destroyed at the same time
		return;
	}
	if (FromSlotId < 0 || FromSlotId >= FromInventory->Size) {
		return;
	}
	if (ToSlotId < 0 || ToSlotId >= Size) {
		return;
	}

	if (FromInventory->Items.ItemsArray[FromSlotId].Count <= 0) {
		return;
	}

	if (!IsItemAllowedAtSlot(ToSlotId, FromInventory->Items.ItemsArray[FromSlotId].ItemRef.ItemDataAsset))
	{
		return;
	}

	Swap(FromInventory->Items.ItemsArray[FromSlotId], Items.ItemsArray[ToSlotId]);
	Items.SyncCounter++;
	FromInventory->Items.SyncCounter++;

	UE_LOG(LogTemp, Warning, TEXT("server on drop from: %d to %d"), FromSlotId, ToSlotId);
	OnRepItems();
	FromInventory->OnRepItems();
}

void UNetworkedInventoryBase::TakeItems(TArray<FItemInstanceStack> ItemsToTake)
{
	if (ItemsToTake.Num() == 0) {
		return;
	}

	int CompletedItems = 0;

	for (FItemInstanceStack& OwnedItem : Items.ItemsArray) {
		if (OwnedItem.ItemRef.ItemDataAsset == nullptr) {
			continue;
		}
		check(OwnedItem.Count > 0);

		for (FItemInstanceStack& ItemToTake : ItemsToTake) {
			if (ItemToTake.Count <= 0 || ItemToTake.ItemRef.ItemDataAsset != OwnedItem.ItemRef.ItemDataAsset) {
				continue;
			}
			// TODO(peto) check other fields as well
			int TakeAmount = FMath::Min(ItemToTake.Count, OwnedItem.Count);

			OwnedItem.Count -= TakeAmount;
			if (OwnedItem.Count <= 0) {
				OwnedItem.ItemRef.ItemDataAsset = nullptr;
				// TODO(peto) maybe set other fields to zero or call Clear
			}

			ItemToTake.Count -= TakeAmount;
			if (ItemToTake.Count <= 0) {
				++CompletedItems;
				if (CompletedItems == ItemsToTake.Num()) {
					OnRepItems();
					return;
				}
			}
		}
	}
	check(false);
}

void UNetworkedInventoryBase::TakeItemCountAtSlot(int32 SlotId, int32 CountToTake)
{
	Items.ItemsArray[SlotId].Count -= CountToTake;
	check(Items.ItemsArray[SlotId].Count >= 0);
	if (Items.ItemsArray[SlotId].Count == 0)
	{
		Items.ItemsArray[SlotId].ItemRef.ItemDataAsset = nullptr;
	}

	OnRepItems();
}

int32 UNetworkedInventoryBase::GetTotalItemCount()
{
	return TotalItemCount;
}

bool UNetworkedInventoryBase::IsFull()
{
	for (int i = 0; i < Items.ItemsArray.Num(); ++i)
	{
		if (Items.ItemsArray[i].ItemRef.ItemDataAsset == nullptr || Items.ItemsArray[i].ItemRef.ItemDataAsset->MaxCountInStack > Items.ItemsArray[i].Count)
		{
			return false;
		}
	}
	return true;
}

void UNetworkedInventoryBase::SetAllowedSlotItems(TArray<TSet<UItemDataAsset*>> NewAllowedItemDataAssetsPerSlot)
{
	bHasSpecificSlotItems = true;
	check(NewAllowedItemDataAssetsPerSlot.Num() <= Size);
	if (NewAllowedItemDataAssetsPerSlot.Num() < Size)
	{
		NewAllowedItemDataAssetsPerSlot.SetNumZeroed(Size);
	}

	bool ModifiedInventory = false;
	AllowedItemDataAssetsPerSlot = NewAllowedItemDataAssetsPerSlot;
	for (int i = 0; i < Size; ++i)
	{
		if (Items.ItemsArray[i].ItemRef.ItemDataAsset != nullptr && !IsItemAllowedAtSlot(i, Items.ItemsArray[i].ItemRef.ItemDataAsset))
		{
			ModifiedInventory = true;
			Items.ItemsArray[i].ItemRef.ItemDataAsset = nullptr;
			Items.ItemsArray[i].Count = 0;
		}
	}
	if (ModifiedInventory)
	{
		OnRepItems();
	}
}

inline bool UNetworkedInventoryBase::IsItemAllowedAtSlot(int SlotIndex, UItemDataAsset* ItemDataAsset)
{
	return !bHasSpecificSlotItems
		|| AllowedItemDataAssetsPerSlot[SlotIndex].Contains(ItemDataAsset);
}

void UNetworkedInventoryBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UNetworkedInventoryBase::OnRepItems()
{
	TotalItemCount = 0;
	
	for (int i = 0; i < Size; ++i)
	{
		if (Items.ItemsArray[i].ItemRef.ItemDataAsset != nullptr)
		{
			TotalItemCount += Items.ItemsArray[i].Count;
		}
	}
	
	if (GetWorld()->WorldType == EWorldType::PIE) {
		auto NetMode = GetWorld()->GetNetMode();
		switch (NetMode) {
			case NM_Client:
				UE_LOG(LogTemp, Warning, TEXT("on rep items client: %d"), GPlayInEditorID - 1);
				break;
			case NM_ListenServer:
				UE_LOG(LogTemp, Warning, TEXT("on rep items listen server"));
				break;
		}
	}
	InventoryChangedDelegate.Broadcast(Items.ItemsArray);
}

void UNetworkedInventoryBase::ClientOnItemDrop(UNetworkedInventoryBase* FromInventory, int FromSlotId, int ToSlotId)
{
	ServerRPCOnItemDrop(FromInventory, FromSlotId, ToSlotId);
}

bool UNetworkedInventoryBase::TryTakeItems(const TArray<FItemInstanceStack>& ItemsToTake)
{
	if (CanTakeItems(ItemsToTake)) {
		TakeItems(ItemsToTake);
		return true;
	}
	return false;
}

bool UNetworkedInventoryBase::TryTakeRequiredResources(const TArray<FRequiredResource>& RequiredResources)
{
	if (CanTakeRequiredResources(RequiredResources))
	{
		TakeRequiredResources(RequiredResources);
		return true;
	}
	return false;
}

bool UNetworkedInventoryBase::CanTakeRequiredResources(const TArray<FRequiredResource>& RequiredResources)
{
	for (const FRequiredResource& RequiredResource : RequiredResources) {
		if (!CanTakeItem(RequiredResource.Item, RequiredResource.Count)) {
			return false;
		}
	}
	return true;
}

void UNetworkedInventoryBase::TakeRequiredResources(const TArray<FRequiredResource>& RequiredResources)
{
	TArray<FItemInstanceStack> RequiredItems;
	for (const FRequiredResource& RequiredResource : RequiredResources)
	{
		RequiredItems.AddZeroed();
		RequiredItems.Last().ItemRef.ItemDataAsset = RequiredResource.Item;
		RequiredItems.Last().Count = RequiredResource.Count;
	}
	TakeItems(RequiredItems);
}

bool UNetworkedInventoryBase::CanTakeItems(const TArray<FItemInstanceStack>& ItemsToTake)
{
	for (const FItemInstanceStack& ItemToTake : ItemsToTake) {
		if (!CanTakeItem(ItemToTake.ItemRef.ItemDataAsset, ItemToTake.Count)) {
			return false;
		}
	}
	return true;
}

bool UNetworkedInventoryBase::CanTakeItem(UItemDataAsset* ItemToTake, int Count)
{
	for (const FItemInstanceStack& OwnedItem : Items.ItemsArray) {
		if (ItemToTake != OwnedItem.ItemRef.ItemDataAsset) {
			continue;
		}

		// TODO(peto) maybe also other fields not only asset

		Count -= OwnedItem.Count;
		if (Count <= 0) {
			return true;
		}
	}
	return false;
}

const TArray<FItemInstanceStack>& UNetworkedInventoryBase::GetItems()
{
	return Items.ItemsArray;
}

TArray<FItemInstanceStack> UNetworkedInventoryBase::GetOneStackInventoryItems()
{
	TMap<UItemDataAsset*, int> OneStackItems;

	for (int i = 0; i < Size; ++i)
	{
		if (Items.ItemsArray[i].ItemRef.ItemDataAsset == nullptr)
		{
			continue;
		}
		OneStackItems.FindOrAdd(Items.ItemsArray[i].ItemRef.ItemDataAsset) += Items.ItemsArray[i].Count;
	}

	TArray<FItemInstanceStack> Result;
		
	for (const auto& ResultStack : OneStackItems)
	{
		Result.Add(FItemInstanceStack{ResultStack.Key, ResultStack.Value});
	}
	return Result;
}

void UNetworkedInventoryBase::InitializeComponent()
{
	Super::InitializeComponent();
	Items.ItemsArray.SetNum(Size);
}

void UNetworkedInventoryBase::SetSize(int NewSize)
{
	Size = NewSize;
}

int UNetworkedInventoryBase::AddItem(const FItemInstance& ItemInstance, int Count)
{
	UItemDataAsset* ItemDataAsset = ItemInstance.ItemDataAsset;
	check(ItemDataAsset != nullptr);
	check(Count > 0);
	int CountLeft = Count;

	for (int i = 0; i < Size; ++i) {
		if (!IsItemAllowedAtSlot(i, ItemInstance.ItemDataAsset))
		{
			continue;
		}
		if (Items.ItemsArray[i].ItemRef.ItemDataAsset == nullptr) {
			Items.ItemsArray[i].ItemRef = ItemInstance;
		}

		if (Items.ItemsArray[i].ItemRef.ItemDataAsset == ItemDataAsset) {
			int AddCount = FMath::Min(CountLeft, ItemDataAsset->MaxCountInStack - Items.ItemsArray[i].Count);
			Items.ItemsArray[i].Count += AddCount;
			CountLeft -= AddCount;
			if (CountLeft <= 0) {
				CountLeft = 0;
				break;
			}
		}
	}
	if (CountLeft != Count) {
		Items.SyncCounter++;
		OnRepItems();
	}

	return Count - CountLeft;
}

int UNetworkedInventoryBase::CanAddItem(const FItemInstance& ItemInstance, int Count)
{
	UItemDataAsset* ItemDataAsset = ItemInstance.ItemDataAsset;
	check(ItemDataAsset != nullptr);
	check(Count > 0);
	int CountLeft = Count;

	for (int i = 0; i < Size; ++i) {
		if (!IsItemAllowedAtSlot(i, ItemInstance.ItemDataAsset))
		{
			continue;
		}
		if (Items.ItemsArray[i].ItemRef.ItemDataAsset == nullptr || Items.ItemsArray[i].ItemRef.ItemDataAsset == ItemDataAsset) {
			int AddCount = FMath::Min(CountLeft, ItemDataAsset->MaxCountInStack - Items.ItemsArray[i].Count);
			CountLeft -= AddCount;
			if (CountLeft <= 0) {
				CountLeft = 0;
				break;
			}
		}
	}
	return Count - CountLeft;
}