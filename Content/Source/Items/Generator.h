#pragma once

#include "CoreMinimal.h"

#include "ItemInstance.h"
#include "Engine/StreamableManager.h"

/**
*
*/

struct FGenerator
{
	FGenerator() = default;

	struct FItemInstance GenerateResource(const FString& AssetName, const EItemRarity Rarity = EItemRarity::Common, bool IsComposed = false);
	struct FItemInstance GenerateConsumable(const FString& AssetName, int ItemLevel, const EItemRarity Rarity = EItemRarity::Common);
	struct FItemInstance GenerateEquip(const FString& AssetName, int ItemLevel, const EItemRarity Rarity = EItemRarity::Common);
	struct FItemInstance GenerateWeapon(const FString& AssetName, int ItemLevel, const EItemRarity Rarity = EItemRarity::Common);

private:

	template<typename DataAssetT>
	DataAssetT* GenerateItem(const FString& AssetName);

	FStreamableManager AssetLoader;
};
