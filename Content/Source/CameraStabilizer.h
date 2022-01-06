// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CameraStabilizer.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SURVIVALFPS_API UCameraStabilizer : public USceneComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	float StabilizationSpeed;

	UPROPERTY(EditAnywhere)
	float StabilizationRotationSpeed;

	UPROPERTY(EditAnywhere)
	bool bStabilize = true;

	USceneComponent* ChildCamera;

public:
	// Sets default values for this component's properties
	UCameraStabilizer();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	void SetCamera(USceneComponent* ChildComponent);
};
