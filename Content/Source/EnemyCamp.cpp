// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCamp.h"

#include "WorldManager.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"


// Sets default values
AEnemyCamp::AEnemyCamp()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AEnemyCamp::BeginPlay()
{
	Super::BeginPlay();
}

void AEnemyCamp::Spawn()
{
	for (int i = 0; i < DesiredNumberOfInstances; ++i)
	{
		FVector DesiredLocation = GetActorLocation() + FVector(FMath::RandPointInCircle(SpawnRange), 0);
		if (!AWorldManager::GetInstance()->GetSurfaceLocation(DesiredLocation))
		{
			continue;
		}
		SpawnedInstances[i] = UAIBlueprintHelperLibrary::SpawnAIFromClass(GetWorld(), EnemyCampData.SpawnableEnemies[0], nullptr, DesiredLocation);
	}
}

// Called every frame
void AEnemyCamp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemyCamp::SetEnemyCamp(const FEnemyCampData& NewEnemyCampData)
{
	EnemyCampData = NewEnemyCampData;
	DesiredNumberOfInstances = FMath::RandRange(EnemyCampData.EnemyCountLowerBound, EnemyCampData.EnemyCountUpperBound);
	SpawnedInstances.SetNum(DesiredNumberOfInstances);

	Spawn();
}

void AEnemyCamp::BeginDestroy()
{
	Super::BeginDestroy();
	
	for (AActor* SpawnedInstance : SpawnedInstances)
	{
		if (SpawnedInstance != nullptr)
		{
			SpawnedInstance->Destroy();
		}
	}
}

