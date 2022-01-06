// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrapplingHookProjectile.generated.h"

UCLASS()
class SURVIVALFPS_API AGrapplingHookProjectile : public AActor
{
	GENERATED_BODY()
	
public:
protected:
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* HookMesh;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;
	
	UPROPERTY()
	class AGrapplingHook* OwnerGrapplingHook;

	UPROPERTY(EditAnywhere)
	float ReturnSpeed;

	float AdjustedReturnSpeed;

	float DistToReturnTarget;

	USceneComponent* ReturnTarget = nullptr;

public:
	// Sets default values for this actor's properties
	AGrapplingHookProjectile();
	virtual void Tick(float DeltaTime) override;
	void SetOwnerGrapplingHook(class AGrapplingHook* GrapplingHook);
	void ReturnBack(USceneComponent* Target, float SpeedMultiplier);
	void Stop();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
