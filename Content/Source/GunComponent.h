// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Engine/EngineTypes.h>
#include "ToolComponent.h"
#include "GunComponent.generated.h"

UENUM()
enum class EFireMode: uint8 {
	AUTOMATIC,
	MANUAL
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIVALFPS_API UGunComponent : public UToolComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	float MinAngleInaccuracy;

	UPROPERTY(EditAnywhere)
	float MaxAngleInaccuracy;

	UPROPERTY(EditAnywhere)
	float WeaponRange;

	UPROPERTY(EditAnywhere)
	float WeaponDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int MagazineSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int MaxMagazines;

	UPROPERTY(BlueprintReadOnly)
	int CurrentMagazineBullets;

	UPROPERTY(BlueprintReadOnly)
	int CurrentReserveBullets;

	UPROPERTY(EditAnywhere)
	EFireMode FireMode;

	UPROPERTY(EditAnywhere)
	float RoundsPerMinute;

	UPROPERTY()
	float ShotCooldown;

	UPROPERTY()
	float NextShotCooldown = 0.0f;

	UPROPERTY(EditAnywhere)
	bool bInfiniteBullets;

	bool bAutoFiring = false;

	UPROPERTY(EditAnywhere)
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere)
	UMaterial* BulletHoleMaterial;

	UPROPERTY(EditAnywhere)
	FVector BulletHoleSize = FVector(4,4,4);

	UPROPERTY(EditAnywhere)
	float BulletLifeSpan = 5;

public:
	USceneComponent* GunMuzzle;

private:
	void CharacterHit(class ABasicCharacter& HitCharacter);
	
protected:
	// Called when the game starts or when spawned
	virtual void InitializeComponent() override;

public:	
	// Called every frame
	UGunComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	FVector GetMuzzleLocation();
	FVector GetFireDirection();
	FVector GetUpDirection();
	float CalculateAngleInaccuracy();

	void TrySpawnShot();

	void FirePressed() override;
	void FireReleased() override;
	bool CanFire() override;
	bool NeedReload() override;
	bool CanReload() override;
	void Reload() override;
	bool MustSendReleaseRPC() override;
		
};
