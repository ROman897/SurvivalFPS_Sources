// Fill out your copyright notice in the Description page of Project Settings.


#include "GrapplingHookProjectile.h"

#include "GrapplingHook.h"
#include "GameFramework/ProjectileMovementComponent.h"


// Sets default values
AGrapplingHookProjectile::AGrapplingHookProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HookMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HookMeshComp"));
	SetRootComponent(HookMesh);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
}

// Called when the game starts or when spawned
void AGrapplingHookProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGrapplingHookProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (ReturnTarget != nullptr)
	{
		float DistToReturnTargetIfPulled = DistToReturnTarget - AdjustedReturnSpeed * DeltaTime;
		float CurrentDist = FVector::Distance(ReturnTarget->GetComponentLocation(), GetActorLocation());

		if (DistToReturnTargetIfPulled < CurrentDist)
		{
			FVector DirectionTowardsReturnTarget = (ReturnTarget->GetComponentLocation() - GetActorLocation()).GetSafeNormal();
			FVector NewLocationIfPulled = ReturnTarget->GetComponentLocation() - DirectionTowardsReturnTarget * DistToReturnTargetIfPulled;
			SetActorLocation(NewLocationIfPulled);
			DistToReturnTarget = DistToReturnTargetIfPulled;
		} else {
			DistToReturnTarget = CurrentDist;
		}
	}
}

void AGrapplingHookProjectile::SetOwnerGrapplingHook(AGrapplingHook* GrapplingHook)
{
	OwnerGrapplingHook = GrapplingHook;
	HookMesh->OnComponentBeginOverlap.AddDynamic(OwnerGrapplingHook, &AGrapplingHook::GrapplingHookProjectileCollided);
}

void AGrapplingHookProjectile::ReturnBack(USceneComponent* Target, float SpeedMultiplier)
{
	ProjectileMovementComponent->StopMovementImmediately();
	ReturnTarget = Target;
	AdjustedReturnSpeed = ReturnSpeed * SpeedMultiplier;
	DistToReturnTarget = FVector::Distance(GetActorLocation(), ReturnTarget->GetComponentLocation());
	HookMesh->SetCollisionProfileName("NoCollision");
}

void AGrapplingHookProjectile::Stop()
{
	ProjectileMovementComponent->StopMovementImmediately();
}

