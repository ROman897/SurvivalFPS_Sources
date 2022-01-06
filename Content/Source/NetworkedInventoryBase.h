// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/ItemInstance.h"
#include "Items/Assets/ComposedItemDataAsset.h"

#include "NetworkedInventoryBase.generated.h"

USTRUCT()
struct FInternalItems {
	GENERATED_BODY()

	UPROPERTY()
	TArray<FItemInstanceStack> ItemsArray;

	// this counter is required because replicated array does not trigger RepNotify method,
	// so every time this array is modified, this variable must be incremented so that
	// clients are notified of the change to array
	UPROPERTY()
	uint32 SyncCounter = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryChangedDelegate, const TArray<FItemInstanceStack>&, NewInventoryItems);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIVALFPS_API UNetworkedInventoryBase : public UActorComponent
{
	GENERATED_BODY()

	friend class UCharacterEquipment;

public:
	
	UPROPERTY(BlueprintAssignable)
	FInventoryChangedDelegate InventoryChangedDelegate;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int Size;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRepItems)
	FInternalItems Items;

	int32 TotalItemCount = 0;

	protected:
	bool bHasSpecificSlotItems = false;
	TArray<TSet<UItemDataAsset*>> AllowedItemDataAssetsPerSlot;

public:
	// Sets default values for this component's properties
	UNetworkedInventoryBase();
	virtual void InitializeComponent() override;

	// must only be called before InitializeComponent is called
	void SetSize(int NewSize);
	
	int AddItem(const FItemInstance& ItemInstance, int Count);

	int CanAddItem(const FItemInstance& ItemInstance, int Count);

	UFUNCTION(BlueprintCallable)
	void ClientOnItemDrop(UNetworkedInventoryBase* FromInventory, int FromSlotId, int ToSlotId);

	bool TryTakeItems(const TArray<FItemInstanceStack>& ItemsToTake);

	bool TryTakeRequiredResources(const TArray<FRequiredResource>& RequiredResources);

	bool CanTakeRequiredResources(const TArray<FRequiredResource>& RequiredResources);

	void TakeRequiredResources(const TArray<FRequiredResource>& RequiredResources);

	UFUNCTION(BlueprintCallable)
	bool CanTakeItems(const TArray<FItemInstanceStack>& ItemsToTake);

	UFUNCTION(BlueprintCallable)
	bool CanTakeItem(UItemDataAsset* ItemToTake, int Count);

	UFUNCTION(BlueprintCallable)
	const TArray<FItemInstanceStack>& GetItems();

	// all item stacks of one type are combined into one mega stack
	TArray<FItemInstanceStack> GetOneStackInventoryItems();

	// function expects that player has enough resources for this, called only if you know there's enough resources!
	void TakeItems(TArray<FItemInstanceStack> ItemsToTake);

	// function expects that player has enough resources for this, called only if you know there's enough resources!
	void TakeItemCountAtSlot(int32 SlotId, int32 CountToTake);

	int32 GetTotalItemCount();
	
	bool IsFull();

	void SetAllowedSlotItems(TArray<TSet<UItemDataAsset*>> NewAllowedItemDataAssetsPerSlot);
	bool IsItemAllowedAtSlot(int SlotIndex, UItemDataAsset* ItemDataAsset);

protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION()
	void OnRepItems();

private:
	UFUNCTION(Server, Reliable)
	void ServerRPCOnItemDrop(UNetworkedInventoryBase* FromInventory, int FromSlotId, int ToSlotId);
	void ServerRPCOnItemDrop_Implementation(UNetworkedInventoryBase* FromInventory, int FromSlotId, int ToSlotId);

};

