#include "ItemPreview.h"

#include "Assets/ItemDataAsset.h"

FString ParseStat(float Value, FString Name)
{
	check(Value != 0);
	FString Result;
	if (Value < 0)
	{
		Result += "-";
	} else
	{
		Result += "+";
	}
	Result += FString::SanitizeFloat(Value) + " " + Name;
	return Result;
}

TArray<FString> UItemPreview::GetDescription(const FItemInstance& ItemInstance)
{
	check(ItemInstance.ItemDataAsset != nullptr);

	TArray<FString> Result;
	FString ItemLevel {"ItemLevel: "};
	ItemLevel += FString::FromInt(ItemInstance.ItemLevel);
	Result.Emplace(MoveTemp(ItemLevel));
	const FConsumableEffectsInstance* ConsumableEffects = ItemInstance.ItemStats.TryGet<FConsumableEffectsInstance>();
	if (ConsumableEffects != nullptr)
	{
		Result.Emplace(ParseStat(ConsumableEffects->Health, "Health"));
		Result.Emplace(ParseStat(ConsumableEffects->Armor, "Armor"));
		Result.Emplace(ParseStat(ConsumableEffects->AttackPowerBoost, "Attack Power"));
		Result.Emplace(ParseStat(ConsumableEffects->DefenseBoost, "Defense"));
	}

	const FEquipEffectsInstance* EquipEffects = ItemInstance.ItemStats.TryGet<FEquipEffectsInstance>();
	if (EquipEffects != nullptr)
	{
		Result.Emplace(ParseStat(EquipEffects->Health, "Health"));
		Result.Emplace(ParseStat(EquipEffects->Armor, "Armor"));
		Result.Emplace(ParseStat(EquipEffects->AttackPower, "Attack Power"));
		Result.Emplace(ParseStat(EquipEffects->Defense, "Defense"));
	}
	
	const FWeaponEffectsInstance* WeaponEffects = ItemInstance.ItemStats.TryGet<FWeaponEffectsInstance>();
	if (WeaponEffects != nullptr)
	{
		FString DamageStat;
		DamageStat += FString::SanitizeFloat(WeaponEffects->DamageFrom) + " - " + FString::SanitizeFloat(WeaponEffects->DamageTo);
		Result.Emplace(MoveTemp(DamageStat));
		
		FString RangeStat {"Range: "};
		RangeStat += FString::SanitizeFloat(WeaponEffects->Range);
		Result.Emplace(MoveTemp(RangeStat));

		FString SpeedStat {"Speed: "};
		SpeedStat += FString::SanitizeFloat(WeaponEffects->Speed);
		Result.Emplace(MoveTemp(SpeedStat));
		
		Result.Emplace(ParseStat(WeaponEffects->Health, "Health"));
		Result.Emplace(ParseStat(WeaponEffects->AttackPower, "Attack Power"));
		Result.Emplace(ParseStat(WeaponEffects->Armor, "Armor"));
		Result.Emplace(ParseStat(WeaponEffects->Defense, "Defense"));
	}

	Result.Emplace(ItemInstance.ItemDataAsset->ItemName);
	
	return Result;
}
