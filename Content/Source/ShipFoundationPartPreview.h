// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ShipData.h"
#include "ShipPartPreviewBase.h"
#include "GameFramework/Actor.h"
#include "ShipFoundationPartPreview.generated.h"

UCLASS()
class SURVIVALFPS_API AShipFoundationPartPreview : public AShipPartPreviewBase
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* SceneRoot;
	
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* PreviewMeshBase;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PreviewCollider;

	int32 CurrentOverlapCount = 0;
	
	UPROPERTY()
	class AShip* CurrentSnapShip = nullptr;

	UPROPERTY(EditDefaultsOnly)
	float NonSnapZRotationPerKeyPress = 90.0f;

	bool bHitValidSnap = false;

	// this contains all information about ship part we are to spawn - tile coords, snap id and data asset
	FSerializedShipFoundationPartData CurrentSerializedShipFoundationPartData;
	
public:
	// Sets default values for this actor's properties
	AShipFoundationPartPreview();
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetDesiredPlacement(FVector CameraLocation, FVector CameraForward, int32 ZAxisRotationPresses) override;
	virtual void LeftMouseClicked(class APlayerCharacter* PlayerCharacter) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	bool CanBuild();
	
	UFUNCTION()
	void OnPreviewColliderBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnPreviewColliderEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void UpdatePreviewMaterial();

	void SetDesiredPlacementWithSnap(FVector CameraLocation, FVector CameraForward,
                                     	int32 ZAxisRotationPresses);
};
