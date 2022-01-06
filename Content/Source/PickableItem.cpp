// Fill out your copyright notice in the Description page of Project Settings.


#include "PickableItem.h"
#include "PlayerCharacter.h"

// Sets default values
APickableItem::APickableItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	SetRootComponent(ItemMesh);
}

// Called when the game starts or when spawned
void APickableItem::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APickableItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APickableItem::SetIsFocused(bool IsFocused)
{
	ItemMesh->SetRenderCustomDepth(IsFocused);
	if (IsFocused) {
		ItemMesh->SetCustomDepthStencilValue(2);
	}
}

void APickableItem::Interact(APlayerCharacter* InteractingPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("interact"));
	if (!HasAuthority()) {
		return;
	}
	int CountAdded = InteractingPlayer->AddItemToBag(InventoryItem);
	if (CountAdded == InventoryItem.Count) {
		Destroy();
	}
	else {
		InventoryItem.Count -= CountAdded;
	}
}

FItemInstanceStack APickableItem::GetInventoryItem()
{
	return InventoryItem;
}

void APickableItem::SetInventoryItem(FItemInstanceStack NewInventoryItem)
{
	InventoryItem = NewInventoryItem;
}

UStaticMeshComponent* APickableItem::GetItemMesh()
{
	return ItemMesh;
}

void APickableItem::DisableCollisionsAndPhysics()
{
	ItemMesh->SetCollisionProfileName("NoCollision");
	ItemMesh->SetSimulatePhysics(false);
}
