// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnvironmentPropDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API UEnvironmentPropDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	class UInGameResourceDataAsset* InGameResourceDataAsset;

	UPROPERTY(EditAnywhere)
	class UStaticMesh* PropStaticMesh;

	UPROPERTY(EditAnywhere)
	TArray<class UMaterialInterface*> Materials;

	// this is only applicable to props without actor
	UPROPERTY(EditAnywhere)
	bool NoCollision;
	
	UPROPERTY(EditAnywhere)
	FVector PropStaticMeshScale = FVector::OneVector;
	
	UPROPERTY(EditAnywhere)
	float CullStartDistance;
	
	UPROPERTY(EditAnywhere)
	float CullCompletelyDistance;

	UPROPERTY(EditAnywhere)
	float MinSlopeAngle;

	UPROPERTY(EditAnywhere)
	float MaxSlopeAngle;

	UPROPERTY(EditAnywhere)
	float MoveAlongTerrainNormalMin;
	
	UPROPERTY(EditAnywhere)
	float MoveAlongTerrainNormalMax;

	UPROPERTY(EditAnywhere)
	bool OrientToTerrain;
	


};
