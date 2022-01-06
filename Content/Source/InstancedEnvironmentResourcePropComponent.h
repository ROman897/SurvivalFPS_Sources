// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "InstancedEnvironmentPropComponent.h"
#include "InstancedEnvironmentResourcePropComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SURVIVALFPS_API UInstancedEnvironmentResourcePropComponent : public UInstancedEnvironmentPropComponent
{
	GENERATED_BODY()

	struct FResourceState
	{
		int32 HitsLeft;
	};

protected:

	// map from primitive element index to counts left
	TMap<int32, FResourceState> InGameResourceStates;

public:
	// Sets default values for this component's properties
	UInstancedEnvironmentResourcePropComponent();
	void ReplacePropByActor(int32 InstanceId);
	void ReclaimPropActor(int32 InstanceId, int32 HitsLeft, const FTransform& InstanceTransform);
	void PropActorDestroyed(int32 InstanceId);
	virtual int32 AddInstance(const FTransform& InstanceTransform) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};
