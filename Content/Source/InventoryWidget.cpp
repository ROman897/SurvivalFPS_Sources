// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryWidget.h"
#include "NetworkedInventoryBase.h"

void UInventoryWidget::SetLinkedNetworkedInventories(const TArray<UNetworkedInventoryBase*>& NetworkedInventories)
{
	LinkedNetworkedInventories = NetworkedInventories;
}
