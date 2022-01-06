#include "Generator.h"

#include "ItemInstance.h"
#include "Assets/ItemDataAsset.h"
#include "Assets/ConsumableItemDataAsset.h"
#include "Assets/EquipableItemDataAsset.h"
#include "Assets/WeaponItemDataAsset.h"
#include "Engine/StreamableManager.h"

float GetEffectInstance(float EffectDefault, const EItemRarity Rarity, int ItemLevel)
{
	check(EffectDefault != 0.0f);
	float Factor = 0.0f;
	switch (Rarity)
	{
		case EItemRarity::Junk: Factor = 0.0f; break;
		case EItemRarity::Common: Factor = 0.5f; break;
		case EItemRarity::Rare: Factor = 1.0f; break;
		case EItemRarity::Epic: Factor = 2.0f; break;
		case EItemRarity::Legendary: Factor = 4.0f; break;
	}
	float Result = EffectDefault * (ItemLevel * Factor);
	float Noise = FMath::RandRange(1.0f, 10.0f + (1.0f * Factor));
	if (Noise < Result && FMath::RandBool())
	{
		Noise *= -1;
	}
	return Result + Noise;
}

FItemInstance FGenerator::GenerateResource(const FString& AssetName, const EItemRarity Rarity, bool IsComposed)
{
	FItemInstance NewItem;
	NewItem.ItemRarity = Rarity;
	if (IsComposed)
	{
		NewItem.ItemDataAsset = GenerateItem<UComposedItemDataAsset>(AssetName);
	} else
	{
		NewItem.ItemDataAsset = GenerateItem<UItemDataAsset>(AssetName);
	}
	NewItem.ItemLevel = 1;
	return NewItem;
}

FItemInstance FGenerator::GenerateConsumable(const FString& AssetName, int ItemLevel, const EItemRarity Rarity)
{
	UConsumableItemDataAsset* Asset = GenerateItem<UConsumableItemDataAsset>(AssetName);
	FConsumableEffectsInstance Effects;
	Effects.Armor = GetEffectInstance(Asset->ItemEffectsRange.ArmorChange, Rarity, ItemLevel);
	Effects.Health = GetEffectInstance(Asset->ItemEffectsRange.HealthChange, Rarity, ItemLevel);
	Effects.DefenseBoost = GetEffectInstance(Asset->ItemEffectsRange.DefenseBoost, Rarity, ItemLevel);
	Effects.AttackPowerBoost = GetEffectInstance(Asset->ItemEffectsRange.AttackPowerBoost, Rarity, ItemLevel);

	FItemInstance NewItem;
	NewItem.ItemRarity = Rarity;
	NewItem.ItemStats.Emplace<FConsumableEffectsInstance>(MoveTemp(Effects));
	NewItem.ItemDataAsset = Asset;
	NewItem.ItemLevel = ItemLevel;
	return NewItem;
}

FItemInstance FGenerator::GenerateEquip(const FString& AssetName, int ItemLevel, const EItemRarity Rarity)
{
	UEquipableItemDataAsset* Asset = GenerateItem<UEquipableItemDataAsset>(AssetName);
	FEquipEffectsInstance Effects;
	Effects.Armor = GetEffectInstance(Asset->ItemEffectsRange.ArmorBoost, Rarity, ItemLevel);
	Effects.Health = GetEffectInstance(Asset->ItemEffectsRange.HealthBoost, Rarity, ItemLevel);
	Effects.Defense = GetEffectInstance(Asset->ItemEffectsRange.DefenseBoost, Rarity, ItemLevel);
	Effects.AttackPower = GetEffectInstance(Asset->ItemEffectsRange.AttackPowerBoost, Rarity, ItemLevel);

	FItemInstance NewItem;
	NewItem.ItemRarity = Rarity;
	NewItem.ItemStats.Emplace<FEquipEffectsInstance>(MoveTemp(Effects));
	NewItem.ItemDataAsset = Asset;
	NewItem.ItemLevel = ItemLevel;
	return NewItem;
}

FItemInstance FGenerator::GenerateWeapon(const FString& AssetName, int ItemLevel, const EItemRarity Rarity)
{
	UWeaponItemDataAsset* Asset = GenerateItem<UWeaponItemDataAsset>(AssetName);
	FWeaponEffectsInstance Effects;
	Effects.Armor = GetEffectInstance(Asset->ItemEffectsRange.ArmorBoost, Rarity, ItemLevel);
	Effects.Health = GetEffectInstance(Asset->ItemEffectsRange.HealthBoost, Rarity, ItemLevel);
	Effects.Defense = GetEffectInstance(Asset->ItemEffectsRange.DefenseBoost, Rarity, ItemLevel);
	Effects.AttackPower = GetEffectInstance(Asset->ItemEffectsRange.AttackPowerBoost, Rarity, ItemLevel);
	Effects.DamageFrom = GetEffectInstance(Asset->WeaponDamage, Rarity, ItemLevel);
	Effects.DamageTo = Effects.DamageFrom + GetEffectInstance(Asset->WeaponDamageRange, Rarity, ItemLevel);

	FItemInstance NewItem;
	NewItem.ItemRarity = Rarity;
	NewItem.ItemStats.Emplace<FWeaponEffectsInstance>(MoveTemp(Effects));
	NewItem.ItemDataAsset = Asset;
	NewItem.ItemLevel = ItemLevel;
	return NewItem;
}

template <typename DataAssetT>
DataAssetT* FGenerator::GenerateItem(const FString& AssetName)
{
	FString Path;
	if constexpr (TIsSame<DataAssetT, UEquipableItemDataAsset>::Value)
	{
		Path += "EquipableItemDataAsset";
	} else if constexpr (TIsSame<DataAssetT, UConsumableItemDataAsset>::Value)
	{
		Path += "ConsumableItemDataAsset";
	} else if constexpr (TIsSame<DataAssetT, UComposedItemDataAsset>::Value)
	{
		Path += "ComposedItemDataAsset";
	} else if constexpr (TIsSame<DataAssetT, UItemDataAsset>::Value)
	{
		Path += "AtomicItemDataAsset";
	} else if constexpr (TIsSame<DataAssetT, UWeaponItemDataAsset>::Value)
	{
		Path += "WeaponItemDataAsset";
	}
	Path += "'/Game/DataAssets/Items/DA_" + AssetName + ".DA_" + AssetName + "'";
	FSoftObjectPath AssetRef(Path);
	DataAssetT* Result = AssetLoader.LoadSynchronous<DataAssetT>(AssetRef);
	check(Result != nullptr);
	return Result;
}
