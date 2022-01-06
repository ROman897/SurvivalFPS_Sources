// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "InteractableInterface.h"
#include "GameFramework/Actor.h"

#include "InGameResource.generated.h"

UCLASS()
class SURVIVALFPS_API AInGameResource : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MainMesh;

	UPROPERTY(EditAnywhere)
	float GatherResourceCooldown;
	
	int HitsLeft;

	class UInGameResourceDataAsset* InGameResourceDataAsset;

	class UInstancedEnvironmentResourcePropComponent* ResourceInstancer;

	int32 InstanceId;

public:
	// Sets default values for this actor's properties
	AInGameResource();

	void InitResource(class UInGameResourceDataAsset* MyInGameResourceDataAsset,
		int MyHitsLeft,
		class UInstancedEnvironmentResourcePropComponent* MyResourceInstancer,
		int32 MyInstanceId);

	UFUNCTION(BlueprintCallable)
	void OnHit(class UNetworkedInventoryBase* TargetInventory, int NumberOfHits = 1);

	void GetReclaimed();

	void Interact(APlayerCharacter* InteractingPlayer) override;
	void SetIsFocused(bool IsFocused) override;
	bool IsContinuousInteraction() override;
	float GetContinuousInteractionCooldown() override;
};
