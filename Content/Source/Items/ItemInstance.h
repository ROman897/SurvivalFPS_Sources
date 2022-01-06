#pragma once

#include "CoreMinimal.h"

#include "ItemInstance.generated.h"

/**
*
*/

UENUM()
enum class EItemRarity {
	Junk,
	Common,
	Rare,
	Epic,
	Legendary
};

struct FConsumableEffectsInstance
{
	float Health = 0;
	float Armor = 0;
	float AttackPowerBoost = 0;
	float DefenseBoost = 0;
};

struct FEquipEffectsInstance
{
	float Armor = 0.0;
	float AttackPower = 0.0;
	float Defense = 0.0;
	float Health = 0.0;
};

struct FWeaponEffectsInstance
{
	float Armor = 0.0;
	float AttackPower = 0.0;
	float Defense = 0.0;
	float Health = 0.0;
	float DamageFrom = 0.0;
	float DamageTo = 0.0;
	float Range = 0.0;
	float Speed = 0.0;
};

USTRUCT(BlueprintType)
struct SURVIVALFPS_API FItemInstance
{
	GENERATED_BODY()

	FItemInstance();
	FItemInstance(class UItemDataAsset*);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UItemDataAsset* ItemDataAsset = nullptr;

	TVariant<FEmptyVariantState, FConsumableEffectsInstance, FEquipEffectsInstance, FWeaponEffectsInstance> ItemStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EItemRarity ItemRarity = EItemRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int ItemLevel = 0;
};

USTRUCT(BlueprintType)
struct SURVIVALFPS_API FItemInstanceStack
{
	GENERATED_BODY()

	FItemInstanceStack();
	FItemInstanceStack(class UItemDataAsset* ItemDataAsset, int Count);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FItemInstance ItemRef;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Count = 0;
};
