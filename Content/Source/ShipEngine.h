// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ShipPart.h"
#include "GameFramework/Actor.h"
#include "ShipEngine.generated.h"

UCLASS()
class SURVIVALFPS_API AShipEngine : public AShipPart
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MainBodyMesh;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PropellerMesh;
	
	UPROPERTY(EditAnywhere)
	float HorsePower;

	UPROPERTY(EditAnywhere)
	float PowerConsumptionMaxThrottlePerSecond;

	// how much relative propeller rotation speed changes per second
	UPROPERTY(EditAnywhere)
	float PropellerRotationSpeedChange;

	UPROPERTY(EditAnywhere)
	float PropellerMaxRotationSpeed;

	float CurrentRelativePropellerRotationSpeed = 0.0f;
	
	float ShipThrottleValue = 0.0f;

public:
	// Sets default values for this actor's properties
	AShipEngine();
	virtual void Tick(float DeltaTime) override;
	float GetHorsePower();
	float GetPowerConsumptionMaxThrottlePerSecond();
	void SetShipThrottleValue(float ThrottleValue);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
