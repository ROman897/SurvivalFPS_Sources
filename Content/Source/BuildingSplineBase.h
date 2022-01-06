// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildingSplineBase.generated.h"

UCLASS()
class SURVIVALFPS_API ABuildingSplineBase : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere)
	class USplineCreatorComponent* ConveyorSplineCreatorComponent;
	
	UPROPERTY(EditAnywhere)
	TMap<int, class UMaterialInterface*> NormalDirectionSplineMaterials;

	UPROPERTY(EditAnywhere)
	TMap<int, class UMaterialInterface*> ReversedDirectionSplineMaterials;

	float SplineLength;
	
public:
	// Sets default values for this actor's properties
	ABuildingSplineBase();
	virtual void Tick(float DeltaTime) override;
	
	void SetSpline(const TArray<FVector>& SplineLocations, FVector StartTangent, FVector EndTangent, bool IsDirectionReversed);
	virtual void SetConnectors(class AConnectionPoint* NewStartConnector, class AConnectionPoint* NewEndConnector);
	virtual void Start();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
