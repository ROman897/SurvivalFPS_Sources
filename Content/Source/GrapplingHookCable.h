// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrapplingHookCable.generated.h"

UCLASS()
class SURVIVALFPS_API AGrapplingHookCable : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	class UCableComponent* CableComponent;

public:
	// Sets default values for this actor's properties
	AGrapplingHookCable();
};
