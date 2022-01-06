#pragma once

#include "CoreMinimal.h"

#include "EquipableItemDataAsset.h"

#include "WeaponItemDataAsset.generated.h"

UCLASS(BlueprintType)
class SURVIVALFPS_API UWeaponItemDataAsset : public UEquipableItemDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float WeaponDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float WeaponDamageRange = 0.0f;

};
