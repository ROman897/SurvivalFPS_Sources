#include "ItemInstance.h"

#include "Assets/ItemDataAsset.h"

FItemInstance::FItemInstance()
{
}

FItemInstance::FItemInstance(UItemDataAsset* ItemDataAsset)
	: ItemDataAsset(ItemDataAsset)
{

}

FItemInstanceStack::FItemInstanceStack()
{
}

FItemInstanceStack::FItemInstanceStack(UItemDataAsset* ItemDataAsset, int Count)
	: ItemRef(ItemDataAsset), Count(Count)
{

}
