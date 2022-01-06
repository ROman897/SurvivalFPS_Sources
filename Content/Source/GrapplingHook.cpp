// Fill out your copyright notice in the Description page of Project Settings.


#include "GrapplingHook.h"

#include "CableComponent.h"
#include "GrapplingHookCable.h"
#include "GrapplingHookProjectile.h"
#include "PickableItem.h"
#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
AGrapplingHook::AGrapplingHook()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GrapplingHookOrigin = CreateDefaultSubobject<USceneComponent>(TEXT("GrapplingHookOriginComp"));
	SetRootComponent(GrapplingHookOrigin);
}

void AGrapplingHook::GrapplingHookProjectileCollided(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == GetParentActor())
	{
		return;
	}
	GrapplingHookProjectile->Stop();
	GrapplingHookProjectile->SetActorLocation(SweepResult.Location);

	GrapplingHookCable->CableComponent->bEnableStiffness = true;
	
	CaughtPickableItem = Cast<APickableItem>(OtherActor);
	
	if (CaughtPickableItem != nullptr)
	{
		// for actors with simulated physics, you first need to disable it
		// and only then attach it so something
		CaughtPickableItem->DisableCollisionsAndPhysics();
		CaughtPickableItem->AttachToActor(GrapplingHookProjectile, FAttachmentTransformRules::KeepWorldTransform);
		RotateCameraTowardGrapplingHook();
		PullBackGrapplingHook(0.2f);
		return;
	}
	
	GrapplingHookProjectile->AttachToActor(OtherActor, FAttachmentTransformRules::KeepWorldTransform);
	GrappledTarget = OtherActor;
	InitialCharacterLaunch();
}

// Called when the game starts or when spawned
void AGrapplingHook::BeginPlay()
{
	Super::BeginPlay();

	MaxRangeSquared = MaxRange * MaxRange;
	OwnerCharacter = Cast<APlayerCharacter>(GetParentActor());
	OwnerCharacterMovement = OwnerCharacter->GetCharacterMovement();
	PreviousGroundFriction = OwnerCharacterMovement->GroundFriction;
	PreviousGravity = OwnerCharacterMovement->GravityScale;
	PreviousAirControl = OwnerCharacterMovement->AirControl;

	GrappleReleaseMinDistanceSquared = GrappleReleaseMinDistance * GrappleReleaseMinDistance;
	GrapplingHookReturnMinDistanceSquared = GrapplingHookReturnMinDistance * GrapplingHookReturnMinDistance;
}

void AGrapplingHook::DestroySpawnedActors()
{
	if (GrapplingHookProjectile != nullptr)
	{
		GrapplingHookProjectile->Destroy();
		GrapplingHookProjectile = nullptr;
	}
	if (GrapplingHookCable != nullptr)
	{
		GrapplingHookCable->Destroy();
		GrapplingHookCable = nullptr;
	}
}

void AGrapplingHook::FireForward(FVector TargetLocation)
{
	if (FVector::DistSquared(TargetLocation, GetActorLocation()) < GrappleReleaseMinDistanceSquared)
	{
		return;
	}
	FVector Direction = TargetLocation - GetActorLocation();

	GrapplingHookProjectile = GetWorld()->SpawnActor<AGrapplingHookProjectile>(GrapplingHookProjectileClass, GetActorLocation(), Direction.Rotation());
	GrapplingHookCable = GetWorld()->SpawnActor<AGrapplingHookCable>(GrapplingHookCableClass, GetActorLocation(), FRotator::ZeroRotator);

	GrapplingHookCable->AttachToActor(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
	
	if (GrapplingHookProjectile != nullptr && GrapplingHookCable != nullptr)
	{
		GrapplingHookState = EGrapplingHookStates::PROJECTILE_TRAVELLING_FORWARD;
		GrapplingHookCable->CableComponent->SetAttachEndToComponent(GrapplingHookProjectile->GetRootComponent());
		GrapplingHookCable->CableComponent->EndLocation = GrapplingHookProjectile->GetActorLocation() - GrapplingHookCable->GetActorLocation();
		GrapplingHookProjectile->SetOwnerGrapplingHook(this);
	} else
	{
		DestroySpawnedActors();
	}
}

void AGrapplingHook::PullBackGrapplingHook(float ReturnSpeedMultiplier)
{
	GrapplingHookState = EGrapplingHookStates::PROJECTILE_TRAVELLING_BACK;
	GrapplingHookProjectile->ReturnBack(GetRootComponent(), ReturnSpeedMultiplier);
}

void AGrapplingHook::TryPickupReturningGrapplingHook()
{
	//RotateCameraTowardGrapplingHook();
	if (FVector::DistSquared(GetActorLocation(), GrapplingHookProjectile->GetActorLocation()) < GrapplingHookReturnMinDistanceSquared)
	{
		if (CaughtPickableItem != nullptr)
		{
			OwnerCharacter->AddItemToBag(CaughtPickableItem->GetInventoryItem());
			CaughtPickableItem->Destroy();
			CaughtPickableItem = nullptr;
		}
		DestroySpawnedActors();
		GrapplingHookState = EGrapplingHookStates::IDLE;
	}
}

void AGrapplingHook::InitialCharacterLaunch()
{
	if (MustReleaseGrappleInAir())
	{
		DestroySpawnedActors();
		GrapplingHookState = EGrapplingHookStates::IDLE;
		return;
	}
	
	GrapplingHookState = EGrapplingHookStates::CHARACTER_GRAPPLING_IN_AIR;
	OwnerCharacterMovement->GroundFriction = GroundFrictionDuringGrappling;
	OwnerCharacterMovement->GravityScale = GravityDuringGrappling;
	OwnerCharacterMovement->AirControl = AirControlDuringGrappling;

	OwnerCharacter->LaunchCharacter(FVector::UpVector * CharacterGrappleInitialLaunchVelocitySize, false, true);
}

void AGrapplingHook::StopCharacterGrapplingInAir()
{
	OwnerCharacterMovement->GroundFriction = PreviousGroundFriction;
	OwnerCharacterMovement->GravityScale = PreviousGravity;
	OwnerCharacterMovement->AirControl = PreviousAirControl;

	PullBackGrapplingHook(1.0f);
}

void AGrapplingHook::CharacterGrappleMovementInAir(float DeltaTime)
{
	if (MustReleaseGrappleInAir())
	{
		StopCharacterGrapplingInAir();
		return;
	}
	FVector DirectionToGrappleTarget = (GrapplingHookProjectile->GetActorLocation() - GetActorLocation()).GetSafeNormal();

	FVector LastMovementInput = OwnerCharacter->GetLastRawMovementInput();
	
	FVector CameraForwardDirection = OwnerCharacter->GetFirstPersonCameraComponent()->GetForwardVector();
	
	FVector CameraForwardMovement = CameraForwardDirection * LastMovementInput.X;
	FVector CameraSideWaysMovement = OwnerCharacter->GetFirstPersonCameraComponent()->GetRightVector() * LastMovementInput.Y;
	
	FVector CameraMovementDirection = (CameraForwardMovement * CameraSideWaysMovement).GetSafeNormal();

	FVector GrappleDirection = (OwnerCharacter->GetFirstPersonCameraComponent()->GetForwardVector() + DirectionToGrappleTarget) / 2;
	OwnerCharacter->LaunchCharacter(((1 - GrappleCharacterMidAirControl) * GrappleDirection + GrappleCharacterMidAirControl * CameraMovementDirection) * CharacterGrappleMidAirLaunchVelocitySizePerSecond * DeltaTime, false, false);
}

void AGrapplingHook::GrapplingProjectileForwardCheckRange()
{
	check(GrapplingHookProjectile != nullptr);
	if (FVector::DistSquared(GetActorLocation(), GrapplingHookProjectile->GetActorLocation()) > MaxRangeSquared)
	{
		PullBackGrapplingHook(1.0f);
	}
}

bool AGrapplingHook::MustReleaseGrappleInAir()
{
	return FVector::DistSquared(GetActorLocation(), GrapplingHookProjectile->GetActorLocation()) < GrappleReleaseMinDistanceSquared;
}

void AGrapplingHook::RotateCameraTowardGrapplingHook()
{
	FRotator LookAtGrappleHook = (GrapplingHookProjectile->GetActorLocation() - OwnerCharacter->GetFirstPersonCameraComponent()->GetComponentLocation()).Rotation();
	OwnerCharacter->GetController()->SetControlRotation(LookAtGrappleHook);
	return;
	
	FRotator CamRotation = OwnerCharacter->GetFirstPersonCameraComponent()->GetComponentRotation();
	// TODO(Roman): do we want to limit look angle?
	
	FRotator ClampedCameraRotation = FRotator(
		FMath::ClampAngle(CamRotation.Pitch, LookAtGrappleHook.Pitch - CharacterGrappleInAirMaxRotationOffset.Pitch, LookAtGrappleHook.Pitch + CharacterGrappleInAirMaxRotationOffset.Pitch),
		FMath::ClampAngle(CamRotation.Yaw, LookAtGrappleHook.Yaw - CharacterGrappleInAirMaxRotationOffset.Yaw, LookAtGrappleHook.Yaw + CharacterGrappleInAirMaxRotationOffset.Yaw),
		FMath::ClampAngle(CamRotation.Roll, LookAtGrappleHook.Roll - CharacterGrappleInAirMaxRotationOffset.Roll, LookAtGrappleHook.Roll + CharacterGrappleInAirMaxRotationOffset.Roll)
		);
	OwnerCharacter->GetController()->SetControlRotation(ClampedCameraRotation);
}

// Called every frame
void AGrapplingHook::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (GrapplingHookState)
	{
		case EGrapplingHookStates::CHARACTER_GRAPPLING_IN_AIR:
			CharacterGrappleMovementInAir(DeltaTime);
			break;
		case EGrapplingHookStates::PROJECTILE_TRAVELLING_FORWARD:
			GrapplingProjectileForwardCheckRange();
			break;
		case EGrapplingHookStates::PROJECTILE_TRAVELLING_BACK:
			TryPickupReturningGrapplingHook();
			break;
		default:
			break;
	}
}

void AGrapplingHook::Fire(FVector TargetLocation)
{
	switch (GrapplingHookState)
	{
		case EGrapplingHookStates::IDLE:
			FireForward(TargetLocation);
			break;
		case EGrapplingHookStates::CHARACTER_GRAPPLING_IN_AIR:
			StopCharacterGrapplingInAir();
			break;
		case EGrapplingHookStates::PROJECTILE_TRAVELLING_FORWARD:
			DestroySpawnedActors();
			GrapplingHookState = EGrapplingHookStates::IDLE;
			break;
	}
}
