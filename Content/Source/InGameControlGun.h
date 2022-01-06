// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "InGameTool.h"
#include "InGameControlGun.generated.h"

UCLASS()
class SURVIVALFPS_API AInGameControlGun : public AInGameTool
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* MainMesh;
	
	UPROPERTY(VisibleAnywhere)
	class UControlGunComponent* ControlGunComponent;

	UPROPERTY(VisibleAnywhere)
	class USceneComponent* MuzzleLocation;

public:
	// Sets default values for this actor's properties
	AInGameControlGun();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
