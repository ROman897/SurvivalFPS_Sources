// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShipPart.generated.h"

UCLASS()
class SURVIVALFPS_API AShipPart : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* SceneRoot;
	
	UPROPERTY(EditAnywhere)
	float Weight;

public:
	// Sets default values for this actor's properties
	AShipPart();
	float GetWeight() const
	{
		return Weight;
	}
};
