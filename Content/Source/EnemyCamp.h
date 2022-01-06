// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BiomeDataAsset.h"
#include "EnemyCamp.generated.h"

UCLASS()
class SURVIVALFPS_API AEnemyCamp : public AActor
{
	GENERATED_BODY()

protected:
	FEnemyCampData EnemyCampData;
	
	int DesiredNumberOfInstances;
	
	TArray<AActor*> SpawnedInstances;

	UPROPERTY(EditAnywhere)
	float SpawnRange;

public:
	// Sets default values for this actor's properties
	AEnemyCamp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void Spawn();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetEnemyCamp(const FEnemyCampData& NewEnemyCampData);

	virtual void BeginDestroy() override;
};
