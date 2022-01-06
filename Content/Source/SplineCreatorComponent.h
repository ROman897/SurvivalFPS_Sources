// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "SplineCreatorComponent.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API USplineCreatorComponent : public USplineComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	float MeshStretchSize;

	UPROPERTY(EditAnywhere)
	class UStaticMesh* ConveyorStaticMesh;

	TArray<class UIndexSplineMeshComponent*> SpawnedSplineMeshes;

	UPROPERTY(EditAnywhere)
	FVector2D SplineScale = FVector2D::UnitVector;

	UPROPERTY(EditAnywhere)
	FName CollisionProfileName;

public:
	void SpawnSpline(const TArray<class UMaterialInterface*>& Materials);
	TArray<class UIndexSplineMeshComponent*> GetSpawnedSplineMeshes();

	void SetMaterials(const TArray<class UMaterialInterface*>& Materials);

};
