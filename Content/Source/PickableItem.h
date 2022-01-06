// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Items/ItemInstance.h"
#include "InteractableInterface.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickableItem.generated.h"

UCLASS()
class SURVIVALFPS_API APickableItem : public AActor,
	public IInteractableInterface
{
	GENERATED_BODY()

private:

	UPROPERTY(EditAnywhere)
	FItemInstanceStack InventoryItem;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* ItemMesh;

public:
	// Sets default values for this actor's properties
	APickableItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void SetIsFocused(bool IsFocused) override;
	virtual void Interact(class APlayerCharacter* InteractingPlayer) override;

	FItemInstanceStack GetInventoryItem();
	void SetInventoryItem(FItemInstanceStack NewInventoryItem);

	class UStaticMeshComponent* GetItemMesh();

	void DisableCollisionsAndPhysics();
};
