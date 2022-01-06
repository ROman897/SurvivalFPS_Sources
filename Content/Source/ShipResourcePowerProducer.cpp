// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipResourcePowerProducer.h"

#include "SharedNetworkedInventory.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.h"
#include "PlayerCharacter.h"
#include "Ship.h"
#include "ShipResourcePowerProducerWidget.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AShipResourcePowerProducer::AShipResourcePowerProducer()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PowerProducerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PowerProducerMeshComp"));
	PowerProducerMesh->SetupAttachment(GetRootComponent());

	InputInventory = CreateDefaultSubobject<USharedNetworkedInventory>(TEXT("InputInventoryComp"));
	InputInventory->SetSize(1);

	InputInventory->InventoryChangedDelegate.AddDynamic(this, &AShipResourcePowerProducer::InventoryChanged);
}

// Called when the game starts or when spawned
void AShipResourcePowerProducer::BeginPlay()
{
	Super::BeginPlay();

	TSet<UItemDataAsset*> AllowedItemDataAssets;

	for (const FConsumableResourceForPower& ConsumableResourceForPower : ConsumableResources)
	{
		AllowedItemDataAssets.Add(ConsumableResourceForPower.ItemDataAsset);
	}

	InputInventory->SetAllowedSlotItems({AllowedItemDataAssets});

	check(PowerProducerWidgetClass);

	FName WidgetName = FName(GetName() + FString("Widget"));
	PowerProducerWidget = CreateWidget<UShipResourcePowerProducerWidget>(UGameplayStatics::GetPlayerController(GetWorld(), 0), PowerProducerWidgetClass, WidgetName);
	check(PowerProducerWidget != nullptr);

	UNetworkedInventoryBase* PlayerBagInventory = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0))->GetNetworkedBagInventory();
	PowerProducerWidget->SetLinkedNetworkedInventories({InputInventory, PlayerBagInventory});
	PowerProducerWidget->SetLinkedShipResourcePowerProducer(this);

	AShip* OwningShip = Cast<AShip>(GetParentActor());
	check(OwningShip != nullptr);
	OwningShip->AddPowerProducer(this);
	
	LastProduceTime = -ProduceCooldown;
}

void AShipResourcePowerProducer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AShip* OwningShip = Cast<AShip>(GetParentActor());
	check(OwningShip != nullptr);
	OwningShip->RemovePowerProducer(this);
}

bool AShipResourcePowerProducer::CanProduce()
{
	if (CurrentConsumableResource.ItemDataAsset == nullptr)
	{
		return false;
	}
	if (GetGameTimeSinceCreation() - LastProduceTime < ProduceCooldown)
	{
		return false;
	}
	return bCachedCanConsume;
}

void AShipResourcePowerProducer::InventoryChanged(const TArray<FItemInstanceStack>& NewInventoryItems)
{
	check(NewInventoryItems.Num() == 1);

	UItemDataAsset* SelectedItemDataAsset = NewInventoryItems[0].ItemRef.ItemDataAsset;
	if (SelectedItemDataAsset == nullptr)
	{
		CurrentConsumableResource.ItemDataAsset = nullptr;
		return;
	}
	
	for (const FConsumableResourceForPower& ConsumableResource : ConsumableResources)
	{
		if (ConsumableResource.ItemDataAsset == SelectedItemDataAsset)
		{
			CurrentConsumableResource = ConsumableResource;
			bCachedCanConsume = InputInventory->CanTakeItem(CurrentConsumableResource.ItemDataAsset, CurrentConsumableResource.ConsumeResourceCount);
			return;
		}
	}

	check(false);
}

// Called every frame
void AShipResourcePowerProducer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float AShipResourcePowerProducer::GetAvailablePower()
{
	if (CanProduce())
	{
		return StoredPowerLeft + CurrentConsumableResource.ConsumeResourceCount * CurrentConsumableResource.ProducedPowerPerUnit;
	}
	return StoredPowerLeft;
}

void AShipResourcePowerProducer::TakePower(float Amount)
{
	if (Amount > StoredPowerLeft)
	{
		check(CanProduce());
		InputInventory->TakeItems({FItemInstanceStack(CurrentConsumableResource.ItemDataAsset, CurrentConsumableResource.ConsumeResourceCount)});
		LastProduceTime = GetGameTimeSinceCreation();
		StoredPowerLeft += CurrentConsumableResource.ConsumeResourceCount * CurrentConsumableResource.ProducedPowerPerUnit;
	}
	StoredPowerLeft -= Amount;
	check(StoredPowerLeft >= 0.0f);
}

void AShipResourcePowerProducer::Interact(APlayerCharacter* InteractingPlayer)
{
	AInGamePlayerController* PlayerController = InteractingPlayer->GetController<AInGamePlayerController>();
	PlayerController->OpenInGameWidget(PowerProducerWidget);
}

void AShipResourcePowerProducer::SetIsFocused(bool IsFocused)
{
	SetMeshFocused(PowerProducerMesh, IsFocused);
}

