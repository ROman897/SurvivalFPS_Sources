// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "InteractableInterface.h"
#include "PowerProducer.h"
#include "ShipPart.h"
#include "GameFramework/Actor.h"
#include "ShipResourcePowerProducer.generated.h"

USTRUCT(BlueprintType)
struct FConsumableResourceForPower
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	class UItemDataAsset* ItemDataAsset = nullptr;

	UPROPERTY(EditAnywhere)
	int ConsumeResourceCount;

	UPROPERTY(EditAnywhere)
	float ProducedPowerPerUnit;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FConsumedPowerResourceChanged, const FConsumableResourceForPower&, FConsumableResourceForPower);

UCLASS()
class SURVIVALFPS_API AShipResourcePowerProducer : public AShipPart, public IPowerProducer, public IInteractableInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FConsumedPowerResourceChanged ConsumedPowerResourceChanged;

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PowerProducerMesh;
	
	UPROPERTY(VisibleAnywhere)
	class USharedNetworkedInventory* InputInventory;

	UPROPERTY(EditAnywhere)
	TArray<FConsumableResourceForPower> ConsumableResources;

	FConsumableResourceForPower CurrentConsumableResource;
	
	UPROPERTY(EditAnywhere)
	float ProduceCooldown;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UShipResourcePowerProducerWidget> PowerProducerWidgetClass;

	UPROPERTY()
	class UShipResourcePowerProducerWidget* PowerProducerWidget;

	float LastProduceTime = 0.0f;

	float StoredPowerLeft = 0.0f;

	bool bCachedCanConsume = false;

public:
	// Sets default values for this actor's properties
	AShipResourcePowerProducer();
	virtual void Tick(float DeltaTime) override;
	virtual float GetAvailablePower() override;
	virtual void TakePower(float Amount) override;

	virtual void Interact(APlayerCharacter* InteractingPlayer) override;
	virtual void SetIsFocused(bool IsFocused) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	bool CanProduce();

	UFUNCTION()
	void InventoryChanged(const TArray<struct FItemInstanceStack>& NewInventoryItems);
};
