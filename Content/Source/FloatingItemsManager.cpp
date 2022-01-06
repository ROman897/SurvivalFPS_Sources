// Fill out your copyright notice in the Description page of Project Settings.


#include "FloatingItemsManager.h"
#include "FloatingPickableItemDataAsset.h"
#include "PickableItem.h"
#include "Items/Assets/ItemDataAsset.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AFloatingItemsManager::AFloatingItemsManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AFloatingItemsManager::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(SpawnFloatingItemsTimerHandle, this, &AFloatingItemsManager::SpawnFloatingItems, 1.0f, true);
}

void AFloatingItemsManager::FloatingPickableItemDestroyed(FIntVector CellCoords, APickableItem* DestroyedPickableItem)
{
	FFloatingItemsCell* FloatingItemsCell = FloatingItemsCells.Find(CellCoords);
	check(FloatingItemsCell != nullptr);

	const int NumberOfRemovedItems = FloatingItemsCell->SpawnedFloatingItems.Remove(DestroyedPickableItem);
	check(NumberOfRemovedItems == 1);
}

void AFloatingItemsManager::GenerateFloatingItemsCell(FIntVector CellCoords)
{
	FFloatingItemsCell& FloatingItemsCell = FloatingItemsCells.FindOrAdd(CellCoords);
	
	FVector BottomLeftCorner = CellCoordsToWorldLocation(CellCoords);
	FVector TopRightCorner = BottomLeftCorner + FVector(FloatingItemsCellWidth, FloatingItemsCellWidth, FloatingItemsCellHeight);

	// to convert between 100x100x100 meters occurence frequency in data asset and volume of
	// our floating items cell
	float OccurencesVolumeMultiplier = (FloatingItemsCellWidth * FloatingItemsCellWidth * FloatingItemsCellHeight) / (10000.0f * 10000.0f * 10000.0f);
	
	for (UFloatingPickableItemDataAsset* SpawnableFloatingItem : SpawnableFloatingItems)
	{
		
		float FloatSpawnCount = FMath::FRandRange(SpawnableFloatingItem->MinOccurencesPer100x100x100M, SpawnableFloatingItem->MaxOccurencesPer100x100x100M) * OccurencesVolumeMultiplier;
		int32 SpawnCountUpperBound = FMath::CeilToInt(FloatSpawnCount);
		int32 SpawnCountLowerBound = FMath::FloorToInt(FloatSpawnCount);
		float RandNumInBounds = FMath::FRandRange(SpawnCountLowerBound, SpawnCountUpperBound);

		int32 SpawnCount;
		if (FloatSpawnCount >= RandNumInBounds)
		{
			SpawnCount = SpawnCountUpperBound;
		} else
		{
			SpawnCount = SpawnCountLowerBound;
		}

		for (int32 InstanceId = 0; InstanceId < SpawnCount; ++InstanceId)
		{
			FVector SpawnLocation = FVector(
				FMath::FRandRange(BottomLeftCorner.X, TopRightCorner.X),
				FMath::FRandRange(BottomLeftCorner.Y, TopRightCorner.Y),
				FMath::FRandRange(BottomLeftCorner.Z, TopRightCorner.Z)
			);

			FItemInstanceStack ItemInstanceStack(SpawnableFloatingItem->ItemDataAsset, 1);
			FloatingItemsCell.FloatingItems.Add({ItemInstanceStack, SpawnLocation});
		}
	}
}

void AFloatingItemsManager::SpawnFloatingItemsFromCell(FIntVector CellCoords)
{
	FFloatingItemsCell& FloatingItemsCell = FloatingItemsCells.FindOrAdd(CellCoords);

	for (const FFloatingItem& FloatingItem : FloatingItemsCell.FloatingItems)
	{
		FRotator RandRotation = FRotator(
			FMath::FRandRange(0.0f, 360.0f),
			FMath::FRandRange(0.0f, 360.0f),
			FMath::FRandRange(0.0f, 360.0f)
		);
		// TODO(Roman): add some proximity check so that floating items arent' spawned too close to each other?
		APickableItem* PickableItem = GetWorld()->SpawnActor<APickableItem>(FloatingItem.ItemInstanceStack.ItemRef.ItemDataAsset->PickableItemClass, FloatingItem.Location, RandRotation);
		if (PickableItem == nullptr)
		{
			return;
		}

		PickableItem->SetInventoryItem(FloatingItem.ItemInstanceStack);
		FloatingItemsCell.SpawnedFloatingItems.Add(PickableItem);
	}
	
	FloatingItemsCell.FloatingItems.Empty();
}

void AFloatingItemsManager::DespawnFloatingItemsFromCell(FIntVector CellCoords)
{
	FFloatingItemsCell& FloatingItemsCell = FloatingItemsCells.FindOrAdd(CellCoords);

	for (APickableItem* PickableItem : FloatingItemsCell.SpawnedFloatingItems)
	{
		if (IsValid(PickableItem))
		{
			FloatingItemsCell.FloatingItems.Add({PickableItem->GetInventoryItem(), PickableItem->GetActorLocation()});
			PickableItem->Destroy();
		}
	}
	FloatingItemsCell.SpawnedFloatingItems.Empty();
}

void AFloatingItemsManager::SpawnFloatingItems()
{
	FIntVector CellAtPlayer = WorldLocationToCellCoords(UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->GetActorLocation());

	TSet<FIntVector> NewSpawnedFloatingItemsCells;
	
	for (int32 CellX = CellAtPlayer.X - NumOfCellsToLoadAroundPlayerXY; CellX <= CellAtPlayer.X + NumOfCellsToLoadAroundPlayerXY; ++CellX)
	{
		for (int32 CellY = CellAtPlayer.Y - NumOfCellsToLoadAroundPlayerXY; CellY <= CellAtPlayer.Y + NumOfCellsToLoadAroundPlayerXY; ++CellY)
		{
			for (int32 CellZ = CellAtPlayer.Z - NumOfCellsToLoadAroundPlayerZ; CellZ <= CellAtPlayer.Z + NumOfCellsToLoadAroundPlayerZ; ++CellZ)
			{
				FIntVector CellCoords(CellX, CellY, CellZ);
				if (!FloatingItemsCells.Contains(CellCoords))
				{
					GenerateFloatingItemsCell(CellCoords);
				}
				NewSpawnedFloatingItemsCells.Add(CellCoords);
			}
		}
	}

	for (FIntVector FloatingItemsCellCoordsToSpawn : NewSpawnedFloatingItemsCells.Difference(SpawnedFloatingItemsCells))
	{
		SpawnFloatingItemsFromCell(FloatingItemsCellCoordsToSpawn);
	}

	for (FIntVector FloatingItemsCellCoordsToDespawn : SpawnedFloatingItemsCells.Difference(NewSpawnedFloatingItemsCells))
	{
		DespawnFloatingItemsFromCell(FloatingItemsCellCoordsToDespawn);
	}
	SpawnedFloatingItemsCells = MoveTemp(NewSpawnedFloatingItemsCells);
}


// Called every frame
void AFloatingItemsManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

