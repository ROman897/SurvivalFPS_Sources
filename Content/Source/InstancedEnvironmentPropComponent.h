// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Components/HierarchicalInstancedStaticMeshComponent.h>

#include "InstancedEnvironmentPropComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SURVIVALFPS_API UInstancedEnvironmentPropComponent : public UHierarchicalInstancedStaticMeshComponent
{
	GENERATED_BODY()

protected:
	const class UEnvironmentPropDataAsset* EnvironmentPropDataAsset;

public:
	// Sets default values for this component's properties
	UInstancedEnvironmentPropComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
public:
	void SetEnvironmentPropDataAsset(const class UEnvironmentPropDataAsset* NewEnvironmentPropDataAsset);
};

