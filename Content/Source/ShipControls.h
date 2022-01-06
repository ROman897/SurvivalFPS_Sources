// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "InteractableInterface.h"
#include "ShipPart.h"
#include "ShipControls.generated.h"

UCLASS()
class SURVIVALFPS_API AShipControls : public AShipPart, public IInteractableInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MainMeshComponent;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* DirectionStickRootComponent;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* ThrottleStickRootComponent;

	UPROPERTY()
	class AShip* OwningShip;

	UPROPERTY(EditAnywhere)
	float RotationStickMaxPitch;
	
	UPROPERTY(EditAnywhere)
	float RotationStickMaxYaw;

	UPROPERTY(EditAnywhere)
	float ThrottleStickMaxRotation;

	UPROPERTY()
	class UShipFloatingMovement* ShipFloatingMovement;

public:
	// Sets default values for this actor's properties
	AShipControls();
	virtual void Tick(float DeltaTime) override;
	virtual void SetIsFocused(bool IsFocused) override;
	virtual void Interact(APlayerCharacter* InteractingPlayer) override;
	void PlayerControlCancelled();
	void AddSteering(float Direction);
	void AddSpeed(float Direction);
	void AddPitch(float Direction);
	void SetShip(class AShip* Ship);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
