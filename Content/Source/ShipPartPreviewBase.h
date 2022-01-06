// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShipPartPreviewBase.generated.h"

UCLASS()
class SURVIVALFPS_API AShipPartPreviewBase : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	float MaxDistanceFromPlayer = 500;

	UPROPERTY(EditAnywhere)
	class UMaterialInterface* PreviewValidMaterial;

	UPROPERTY(EditAnywhere)
	class UMaterialInterface* PreviewInvalidMaterial;

	class UShipPartDataAsset* ShipPartDataAsset;
	
public:	
	// Sets default values for this actor's properties
	AShipPartPreviewBase();
	virtual void Tick(float DeltaTime) override;
	virtual void SetDesiredPlacement(FVector CameraLocation, FVector CameraForward, int32 ZAxisRotationPresses);
	FHitResult LineTrace(const FVector& StartTrace, const FVector& Direction, float TraceRange, ECollisionChannel CollisionChannel) const;
	FHitResult LineTraceMaxDistFromPlayer(const FVector& StartTrace, const FVector& Direction, ECollisionChannel CollisionChannel) const;
	TArray<FHitResult> LineTraceMulti(const FVector& StartTrace, const FVector& Direction, float TraceRange, ECollisionChannel CollisionChannel);
	TArray<FHitResult> LineTraceMultiMaxDistFromPlayer(const FVector& StartTrace, const FVector& Direction, ECollisionChannel CollisionChannel);
	
	virtual void LeftMouseClicked(class APlayerCharacter* PlayerCharacter);
	virtual void OnMouseWheel(float Val);

	void SetShipPartDataAsset(UShipPartDataAsset* MyShipPartDataAsset)
	{
		ShipPartDataAsset = MyShipPartDataAsset;
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
