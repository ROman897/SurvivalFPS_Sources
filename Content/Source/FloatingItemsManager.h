// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items/ItemInstance.h"

#include "FloatingItemsManager.generated.h"

struct FFloatingItem
{
	FItemInstanceStack ItemInstanceStack;
	FVector Location;
};

USTRUCT()
struct FFloatingItemsCell
{
	GENERATED_BODY()
	TArray<FFloatingItem> FloatingItems;
	
	UPROPERTY()
	TSet<class APickableItem*> SpawnedFloatingItems;
};

UCLASS()
class SURVIVALFPS_API AFloatingItemsManager : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly)
	TArray<class UFloatingPickableItemDataAsset*> SpawnableFloatingItems;

	TMap<FIntVector, FFloatingItemsCell> FloatingItemsCells;

	TSet<FIntVector> SpawnedFloatingItemsCells;

	UPROPERTY(EditDefaultsOnly)
	float FloatingItemsCellWidth;
	
	UPROPERTY(EditDefaultsOnly)
	float FloatingItemsCellHeight;

	UPROPERTY(EditDefaultsOnly)
	int32 NumOfCellsToLoadAroundPlayerXY;
	
	UPROPERTY(EditDefaultsOnly)
	int32 NumOfCellsToLoadAroundPlayerZ;

	FTimerHandle SpawnFloatingItemsTimerHandle;

public:
	// Sets default values for this actor's properties
	AFloatingItemsManager();
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void FloatingPickableItemDestroyed(FIntVector CellCoords, class APickableItem* DestroyedPickableItem);

	void GenerateFloatingItemsCell(FIntVector CellCoords);
	void SpawnFloatingItemsFromCell(FIntVector CellCoords);
	void DespawnFloatingItemsFromCell(FIntVector CellCoords);

	void SpawnFloatingItems();

	FVector CellCoordsToWorldLocation(FIntVector CellCoords)
	{
		return FVector(CellCoords.X * FloatingItemsCellWidth, CellCoords.Y * FloatingItemsCellWidth, CellCoords.Z * FloatingItemsCellHeight);
	}

	FIntVector WorldLocationToCellCoords(FVector Location)
	{
		return FIntVector(Location.X / FloatingItemsCellWidth, Location.Y / FloatingItemsCellWidth, Location.Z / FloatingItemsCellHeight);
	}
};

