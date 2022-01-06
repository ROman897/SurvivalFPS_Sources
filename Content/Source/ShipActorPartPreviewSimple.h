// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ShipPartPreviewBase.h"
#include "ShipActorPartPreviewSimple.generated.h"

USTRUCT(BlueprintType)
struct FMeshVariation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	UStaticMesh* PreviewMesh;

	UPROPERTY(EditAnywhere)
	FVector PreviewMeshScale;

	UPROPERTY(EditAnywhere)
	FVector PreviewColliderScale;

	UPROPERTY(EditAnywhere)
	FRotator MeshRotation;

	UPROPERTY(EditAnywhere)
	FVector PreviewMeshLocationOffset;
	
	UPROPERTY(EditAnywhere)
	FVector PreviewColliderLocationOffset;
};

UCLASS()
class SURVIVALFPS_API AShipActorPartPreviewSimple : public AShipPartPreviewBase
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
	class AShip* HitShip = nullptr;

	UPROPERTY(EditDefaultsOnly)
	float ZRotationPerKeyPress = 90.0f;
	
public:
	// Sets default values for this actor's properties
	AShipActorPartPreviewSimple();
	virtual void Tick(float DeltaTime) override;
	virtual void SetDesiredPlacement(FVector CameraLocation, FVector CameraForward, int32 ZAxisRotationPresses) override;
	virtual void LeftMouseClicked(class APlayerCharacter* PlayerCharacter) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void SetDesiredPlacementLocation(FVector CameraLocation, FVector CameraForward, int32 ZAxisRotationPresses);

	bool CanBuild();
	
	UFUNCTION()
	void OnPreviewColliderBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnPreviewColliderEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void UpdatePreviewMaterial();

	void HitNothing(FVector CameraLocation, FVector CameraForward, const FRotator& DesiredFreeformRotation);
};
