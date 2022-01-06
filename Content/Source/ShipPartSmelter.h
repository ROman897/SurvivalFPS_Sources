// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ShipPartAutoCrafter.h"
#include "ShipPartSmelter.generated.h"

UCLASS()
class SURVIVALFPS_API AShipPartSmelter : public AShipPartAutoCrafter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* SmelterMesh;

public:
	// Sets default values for this actor's properties
	AShipPartSmelter();

};
