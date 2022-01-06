// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "EnvironmentPropsInstancer.generated.h"

UCLASS()
class SURVIVALFPS_API AEnvironmentPropsInstancer : public AActor
{
	GENERATED_BODY()

protected:
	static AEnvironmentPropsInstancer* Instance;

	TMap<const class UEnvironmentPropDataAsset*, class UInstancedEnvironmentPropComponent*> HierarchicalInstancedMeshComponents;

public:
	// Sets default values for this actor's properties
	AEnvironmentPropsInstancer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	static AEnvironmentPropsInstancer* GetInstance();
	
	void RegisterEnvironmentProp(const class UEnvironmentPropDataAsset* EnvironmentPropDataAsset);

	void AddInstance(const class UEnvironmentPropDataAsset* EnvironmentPropDataAsset, const FTransform& Transform);
};