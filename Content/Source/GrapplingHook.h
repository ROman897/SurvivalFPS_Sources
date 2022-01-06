// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrapplingHook.generated.h"

UCLASS()
class SURVIVALFPS_API AGrapplingHook : public AActor
{
	GENERATED_BODY()

	enum class EGrapplingHookStates
	{
		IDLE,
		PROJECTILE_TRAVELLING_FORWARD,
		CHARACTER_GRAPPLING_IN_AIR,
		PROJECTILE_TRAVELLING_BACK
	};

protected:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* GrapplingHookOrigin;

	UPROPERTY(EditAnywhere)
	float MaxRange;

	float MaxRangeSquared = 0.0f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AGrapplingHookProjectile> GrapplingHookProjectileClass;

	UPROPERTY()
	class AGrapplingHookProjectile* GrapplingHookProjectile;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AGrapplingHookCable> GrapplingHookCableClass;

	UPROPERTY()
	class AGrapplingHookCable* GrapplingHookCable;

	EGrapplingHookStates GrapplingHookState = EGrapplingHookStates::IDLE;

	AActor* GrappledTarget;

	UPROPERTY(EditAnywhere)
	float GroundFrictionDuringGrappling = 0.0f;

	UPROPERTY(EditAnywhere)
	float GravityDuringGrappling = 0.0f;

	UPROPERTY(EditAnywhere)
	float AirControlDuringGrappling = 0.0f;

	UPROPERTY(EditAnywhere)
	float CharacterGrappleInitialLaunchVelocitySize;

	UPROPERTY(EditAnywhere)
	float CharacterGrappleMidAirLaunchVelocitySizePerSecond;

	UPROPERTY(EditAnywhere)
	float GrappleReleaseMinDistance;

	UPROPERTY(EditAnywhere)
	float GrappleCharacterMidAirControl;

	float GrappleReleaseMinDistanceSquared;

	UPROPERTY(EditAnywhere)
	float GrapplingHookReturnMinDistance;

	float GrapplingHookReturnMinDistanceSquared;

	class APickableItem* CaughtPickableItem = nullptr;

	float PreviousGroundFriction;
	float PreviousGravity;
	float PreviousAirControl;

	UPROPERTY(EditAnywhere)
	FRotator CharacterGrappleInAirMaxRotationOffset;

	class APlayerCharacter* OwnerCharacter;
	class UCharacterMovementComponent* OwnerCharacterMovement;

public:
	// Sets default values for this actor's properties
	AGrapplingHook();
	virtual void Tick(float DeltaTime) override;
	void Fire(FVector TargetLocation);
	float GetMaxRange() const
	{
		return MaxRange;
	}

	UFUNCTION()
	void GrapplingHookProjectileCollided(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void DestroySpawnedActors();
	void FireForward(FVector TargetLocation);
	void PullBackGrapplingHook(float ReturnSpeedMultiplier);
	void TryPickupReturningGrapplingHook();
	void InitialCharacterLaunch();
	void StopCharacterGrapplingInAir();
	void CharacterGrappleMovementInAir(float DeltaTime);
	void GrapplingProjectileForwardCheckRange();
	bool MustReleaseGrappleInAir();
	void RotateCameraTowardGrapplingHook();
};

