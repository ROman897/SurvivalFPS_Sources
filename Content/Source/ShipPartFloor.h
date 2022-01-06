// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ShipPart.h"
#include "GameFramework/Actor.h"
#include "ShipPartFloor.generated.h"

UCLASS()
class SURVIVALFPS_API AShipPartFloor : public AShipPart
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* FloorMesh;

public:
	// Sets default values for this actor's properties
	AShipPartFloor();
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
