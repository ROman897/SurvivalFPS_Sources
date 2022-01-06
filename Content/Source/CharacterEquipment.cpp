// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterEquipment.h"
#include "PlayerCharacter.h"
#include "NetworkedInventoryBase.h"
#include "Items/ItemInstance.h"
#include "Items/Assets/EquipableItemDataAsset.h"
#include "ToolComponent.h"
#include "InGameTool.h"

#include <Net/UnrealNetwork.h>


// Sets default values for this component's properties
UCharacterEquipment::UCharacterEquipment()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UCharacterEquipment::SwitchToolUp()
{
	SwitchTool(1);
}

void UCharacterEquipment::SwitchToolDown()
{
	SwitchTool(-1);
}

// Called when the game starts
void UCharacterEquipment::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<APlayerCharacter>(GetOwner());
	check(OwnerCharacter);
	LinkedEquipmentInventory = OwnerCharacter->GetNetworkedEquipmentInventory();
	check(LinkedEquipmentInventory);
	LinkedEquipmentInventory->InventoryChangedDelegate.AddDynamic(this, &UCharacterEquipment::EquipmentChanged);
}

void UCharacterEquipment::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UCharacterEquipment, PrimaryToolSlotId, COND_SkipOwner);
}

void UCharacterEquipment::EquipmentChanged(const TArray<FItemInstanceStack>& NewInventoryItems)
{
	if (GetWorld()->WorldType == EWorldType::PIE) {
		auto NetMode = GetWorld()->GetNetMode();
		switch (NetMode) {
			case NM_Client:
				UE_LOG(LogTemp, Warning, TEXT("equipment changed on client: %d"), GPlayInEditorID - 1);
				break;
			case NM_ListenServer:
				UE_LOG(LogTemp, Warning, TEXT("equipment changed on listen server"));
				break;
		}
	}
	ActiveToolChanged();
}

void UCharacterEquipment::DestroyTool()
{
	OwnerCharacter->PrimaryTool->GetOwner()->Destroy();
	OwnerCharacter->PrimaryTool = nullptr;
	CurrentToolClass = nullptr;
}

void UCharacterEquipment::SwitchTool(int Direction)
{
	PrimaryToolSlotId += Direction;
	if (PrimaryToolSlotId == MAX_TOOLS) {
		PrimaryToolSlotId = 0;
	} else {
		if (PrimaryToolSlotId == -1) {
			PrimaryToolSlotId = MAX_TOOLS - 1;
		}
	}

	ActiveToolChanged();
}

void UCharacterEquipment::ActiveToolChanged()
{
	PrimaryToolDataAsset = Cast<UEquipableItemDataAsset>(LinkedEquipmentInventory->Items.ItemsArray[PrimaryToolSlotId].ItemRef.ItemDataAsset);
	APlayerCharacter* OwnerChar = Cast<APlayerCharacter>(GetOwner());
	check(OwnerChar);

	if (PrimaryToolDataAsset != nullptr && PrimaryToolDataAsset->SpawnableClass != CurrentToolClass) {
		if (OwnerCharacter->PrimaryTool != nullptr) {
			DestroyTool();
		}

		CurrentToolClass = PrimaryToolDataAsset->SpawnableClass;
		check(CurrentToolClass);
		AInGameTool* ToolInstance = GetWorld()->SpawnActor<AInGameTool>(CurrentToolClass);
		ToolInstance->AttachToComponent(OwnerChar->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ToolInstance->AttachmentSocketName);
		OwnerChar->PrimaryTool = &ToolInstance->GetToolComponent();
		ToolInstance->GetToolComponent().SetOwner(OwnerChar);
		return;
	}

	if (PrimaryToolDataAsset == nullptr && OwnerChar->PrimaryTool != nullptr) {
		DestroyTool();
	}
}

void UCharacterEquipment::ToolSwitched()
{
	UE_LOG(LogTemp, Warning, TEXT("weapon switched on rep"));
	ActiveToolChanged();
}

void UCharacterEquipment::ServerRPCSwitchTool_Implementation(int NewPrimaryToolSlotId)
{
	if (Cast<APawn>(GetOwner())->IsLocallyControlled()) {
		return;
	}
	PrimaryToolSlotId = NewPrimaryToolSlotId;
	ActiveToolChanged();
}
