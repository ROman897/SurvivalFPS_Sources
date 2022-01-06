// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/ItemInstance.h"
#include "CharacterEquipment.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIVALFPS_API UCharacterEquipment : public UActorComponent
{
	GENERATED_BODY()

	enum EquipmentSlot {
		SLOT_WEAPON0 = 0
	};

protected:
	UPROPERTY(BlueprintReadOnly)
	class UEquipableItemDataAsset* PrimaryToolDataAsset;

	TSubclassOf<class AInGameTool> CurrentToolClass;
	class APlayerCharacter* OwnerCharacter;

	UPROPERTY(ReplicatedUsing = ToolSwitched)
	int PrimaryToolSlotId = 0;

	static constexpr int MAX_TOOLS = 2;

	class UNetworkedInventoryBase* LinkedEquipmentInventory;

public:
	// Sets default values for this component's properties
	UCharacterEquipment();

	void SwitchToolUp();
	void SwitchToolDown();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION()
	void EquipmentChanged(const TArray<FItemInstanceStack>& NewInventoryItems);

	UFUNCTION()
	void ToolSwitched();

	UFUNCTION(Server, Reliable)
	void ServerRPCSwitchTool(int NewPrimaryToolSlotId);
	void ServerRPCSwitchTool_Implementation(int NewPrimaryToolSlotId);

	void DestroyTool();
	void SwitchTool(int Direction);

	void ActiveToolChanged();
};
