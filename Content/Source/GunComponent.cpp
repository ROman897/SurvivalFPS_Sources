// Fill out your copyright notice in the Description page of Project Settings.


#include "GunComponent.h"

#include "BasicCharacter.h"

#include <Math/UnrealMathUtility.h>
#include <Kismet/GameplayStatics.h>

#include "Ship.h"
#include "Engine/DecalActor.h"

#define COLLISION_WEAPON		ECC_GameTraceChannel1

// Sets default values
UGunComponent::UGunComponent()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
	static ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("Material'/Game/Materials/bullet_hole_Mat.bullet_hole_Mat'"));
	BulletHoleMaterial = Material.Object;
}

FVector UGunComponent::GetMuzzleLocation()
{
	return GunMuzzle->GetComponentLocation();
}

FVector UGunComponent::GetFireDirection()
{
	return GunMuzzle->GetRightVector().GetSafeNormal();
}

FVector UGunComponent::GetUpDirection()
{
	return GunMuzzle->GetUpVector().GetSafeNormal();
}

// Called when the game starts or when spawned
void UGunComponent::InitializeComponent()
{
	Super::InitializeComponent();
	CurrentMagazineBullets = MagazineSize;
	CurrentReserveBullets = MagazineSize * (MaxMagazines - 1);
	float RoundsPerSecond = RoundsPerMinute / 60.0f;
	ShotCooldown = 1 / (RoundsPerSecond);
}

// Called every frame
void UGunComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (NextShotCooldown > 0.0f) {
		NextShotCooldown -= DeltaTime;
	}

	if (bAutoFiring) {
		TrySpawnShot();
	}
}

void UGunComponent::CharacterHit(ABasicCharacter& HitCharacter)
{
	if (!HitCharacter.HasAuthority()) {
		return;
	}
	HitCharacter.OnHit(WeaponDamage);
}

float UGunComponent::CalculateAngleInaccuracy()
{
	float CharacterInaccuracy = OwnerCharacter->CalculateCharacterInaccuracy();
	return MinAngleInaccuracy + (MaxAngleInaccuracy - MinAngleInaccuracy) * CharacterInaccuracy;
}

void UGunComponent::TrySpawnShot()
{
	if (!CanFire()) {
		return;
	}

	if (FireSound != nullptr) {
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetOwner()->GetActorLocation());
	}

	OwnerCharacter->ToolFired();

	--CurrentMagazineBullets;

	NextShotCooldown += ShotCooldown;

	FVector StartTrace = GetMuzzleLocation();
	FVector ShootDir = GetFireDirection();

	float TotalAngleInaccuracy = CalculateAngleInaccuracy();
	FVector RandomizedShootDir = FMath::VRandCone(ShootDir, TotalAngleInaccuracy * (PI / 180.0f));
	FVector EndTrace = StartTrace + RandomizedShootDir * WeaponRange;

	// Check for impact
	const FHitResult Impact = LineTrace(StartTrace, EndTrace, COLLISION_WEAPON);

	// Make bullet hole
	//UGameplayStatics::SpawnDecalAtLocation(GetWorld(), BulletHoleMaterial, BulletHoleSize, Impact.Location, Impact.Normal.Rotation(),BulletLifeSpan);
	

	AActor* HitActor = Impact.GetActor();
	if (HitActor != nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("hit actor: %s"), *HitActor->GetName());
	}

	// TODO(Roman): ignore character meshe that holds this weapon?
	if (HitActor == nullptr || HitActor == GetOwner()) {
		return;
	}

	ABasicCharacter* HitCharacter = Cast<ABasicCharacter>(HitActor);

	if (HitCharacter != nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("cast to basic character success"));
		CharacterHit(*HitCharacter);
		return;
	}

	AShip* HitShip = Cast<AShip>(HitActor);
	if (HitShip != nullptr)
	{
		FTransform DestructionCapsuleTransform;
		DestructionCapsuleTransform.SetLocation(Impact.Location);
		DestructionCapsuleTransform.SetRotation(ShootDir.ToOrientationQuat());
		HitShip->ShipHitWithBullet(Impact.Component.Get(), DestructionCapsuleTransform, 8, 2);
		
	}
}

void UGunComponent::FirePressed()
{
	TrySpawnShot();
	if (FireMode == EFireMode::AUTOMATIC) {
		bAutoFiring = true;
	}
}

void UGunComponent::FireReleased()
{
	bAutoFiring = false;
}

bool UGunComponent::CanFire()
{
	return (bInfiniteBullets || CurrentMagazineBullets > 0) && NextShotCooldown <= 0.0f;
}

bool UGunComponent::NeedReload()
{
	return !bInfiniteBullets && CurrentMagazineBullets <= 0;
}

bool UGunComponent::CanReload()
{
	return CurrentReserveBullets > 0;
}

void UGunComponent::Reload()
{
	int BulletsToTake = FMath::Min(CurrentReserveBullets, MagazineSize);
	CurrentReserveBullets -= BulletsToTake;
	CurrentMagazineBullets = BulletsToTake;
}

bool UGunComponent::MustSendReleaseRPC()
{
	return FireMode == EFireMode::AUTOMATIC;
}
