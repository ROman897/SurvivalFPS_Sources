// Fill out your copyright notice in the Description page of Project Settings.


#include "Ship.h"

#include "IngameGameState.h"
#include "MyGameInstance.h"
#include "PowerProducer.h"
#include "ShipControls.h"
#include "Components/BoxComponent.h"
#include "ShipFloatingMovement.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ShipPart.h"
#include "ShipEngine.h"
#include "SNodePanel.h"
#include "Items/Assets/ShipPartDataAsset.h"


// Sets default values
AShip::AShip()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoundaryBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CustomRootComp"));
	SetRootComponent(BoundaryBoxComponent);

	ShipFloatingMovement = CreateDefaultSubobject<UShipFloatingMovement>(TEXT("ShipFloatingMovementComp"));
}

// Called when the game starts or when spawned
void AShip::BeginPlay()
{
	Super::BeginPlay();

	FillAllSnapDefinitionsArray();

	SpawnStartingShipFoundations();

	check(GetWorld() != nullptr);
	check(GetWorld()->GetGameState<AIngameGameState>() != nullptr);
	GetWorld()->GetGameState<AIngameGameState>()->AllPlayerShips.Add(this);
}

void AShip::CalculateEngineStats()
{
	RequiredPowerForFullThrottlePerSecond = 0.0f;
	for (AShipEngine* ShipEngine : ShipEngines)
	{
		RequiredPowerForFullThrottlePerSecond += ShipEngine->GetPowerConsumptionMaxThrottlePerSecond();
	}

	float TotalHorsePower = 0.0f; 
	for (AShipEngine* ShipEngine : ShipEngines)
	{
		TotalHorsePower += ShipEngine->GetHorsePower();
	}

	float TotalWeight = 0.0f;
	for (AShipPart* ShipPart : ShipPartChildActors)
	{
		TotalWeight += ShipPart->GetWeight();
	}

	ShipFloatingMovement->CalculateEngineStats(TotalHorsePower, TotalWeight);
}

void AShip::SpawnStartingShipFoundations()
{
	for (int X = -StartFloorHalfLength; X <= StartFloorHalfLength; ++X)
	{
		for (int Y = -StartFloorHalfWidth; Y <= StartFloorHalfWidth; ++Y)
		{
			SpawnShipFoundationPart(FSerializedShipFoundationPartData {
				FIntVector(X, Y, 0),
				0,
				StartFloorShipPartDataAsset
			});
		}
	}
}

void AShip::SpawnShipFoundationPart(const FSerializedShipFoundationPartData& ShipFoundationPartData)
{
	check(ShipFoundationPartData.ShipPartDataAsset != nullptr);
	check(ShipFoundationPartData.ShipPartDataAsset->ShipPartType == EShipPartTypes::FOUNDATION);
	
	FIntVector TileCoords = ShipFoundationPartData.ShipTileCoords;
	
	FName Name = FName(FString("ShipFoundationPartStaticMesh_")
		+ ShipFoundationPartData.ShipPartDataAsset->GetName()
		+ FString("_") + TileCoords.ToString()
		+ FString("_")
		+ FString::FromInt(ShipFoundationPartData.TileSnapId)
	);

	UStaticMeshComponent* ShipPartDestructibleMeshComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), Name);
	check(ShipPartDestructibleMeshComponent != nullptr);
	ShipPartDestructibleMeshComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
	ShipPartDestructibleMeshComponent->RegisterComponent();
	AddInstanceComponent(ShipPartDestructibleMeshComponent);

	const FShipPartSnapColliderDefinition& ShipPartSnapColliderDefinition = FindShipPartSnapColliderDefinition(static_cast<int>(ShipFoundationPartData.ShipPartDataAsset->ShipPartSnapType), ShipFoundationPartData.TileSnapId);
	
	ShipPartDestructibleMeshComponent->SetRelativeLocation(GetRelativeTileLocation(TileCoords) + ShipPartSnapColliderDefinition.SnapPointOffsetFromTile);
	ShipPartDestructibleMeshComponent->SetRelativeRotation(ShipPartSnapColliderDefinition.SnapPointRelativeRotation);
	ShipPartDestructibleMeshComponent->SetRelativeScale3D(ShipPartSnapColliderDefinition.SnapPointScale);
	ShipPartDestructibleMeshComponent->SetMobility(EComponentMobility::Movable);
	ShipPartDestructibleMeshComponent->SetStaticMesh(ShipFoundationPartData.ShipPartDataAsset->ShipPartStaticMesh);
	//ShipPartDestructibleMeshComponent->SetGenerateOverlapEvents(false);
	for (int i = 0; i < ShipFoundationPartData.ShipPartDataAsset->Materials.Num(); ++i)
	{
		ShipPartDestructibleMeshComponent->SetMaterial(i, ShipFoundationPartData.ShipPartDataAsset->Materials[i]);
	}

	FShipTile& ShipTile = FindOrAddShipTile(TileCoords);

	int SnapTypeIndex = static_cast<int>(ShipFoundationPartData.ShipPartDataAsset->ShipPartSnapType);
	check(SnapTypeIndex < static_cast<int>(EShipPartSnapTypes::ENUM_COUNT));
	FShipPartsOneSnapType& ShipPartsOneSnapType = ShipTile.ShipParts[SnapTypeIndex];

	ShipPartsOneSnapType.ShipFoundationParts.AddUninitialized();
	FShipFoundationPart& ShipFoundationPart = ShipPartsOneSnapType.ShipFoundationParts.Last();
	ShipFoundationPart.ShipPartDataAsset = ShipFoundationPartData.ShipPartDataAsset;
	ShipFoundationPart.TileSnapId = ShipFoundationPartData.TileSnapId;
	ShipFoundationPart.ShipPartDestructibleMeshComponent = ShipPartDestructibleMeshComponent;

	FoundationMeshToSnapHandle.Emplace(ShipPartDestructibleMeshComponent, FShipPartSnapHandle {SnapTypeIndex, TileCoords, ShipFoundationPartData.TileSnapId});

	TileFoundationsChanged(ShipFoundationPartData.ShipTileCoords);
}

FIntVector AShip::GetTileCoordsFromWorldLocation(FVector WorldLocation)
{
	const FVector RelativeLocation = GetActorTransform().InverseTransformPosition(WorldLocation);
	return FIntVector(FMath::RoundToInt(RelativeLocation.X / TILE_LENGTH), FMath::RoundToInt(RelativeLocation.Y / TILE_LENGTH), FMath::RoundToInt(RelativeLocation.Z / TILE_LENGTH));
}

void AShip::UpdateSnapCollidersAtTile(FIntVector TileCoords)
{
	FShipTile& ShipTile = FindOrAddShipTile(TileCoords);
	
	for (int ShipPartSnapType = 0; ShipPartSnapType < SHIP_PART_SNAP_TYPES_COUNT; ++ShipPartSnapType)
	{
		FShipPartsOneSnapType& ShipPartsOneSnapType = ShipTile.ShipParts[ShipPartSnapType];
		for (int SnapId = 0; SnapId < ShipPartsOneSnapType.SpawnableShipFoundationSnapColliders.Num(); ++SnapId)
		{
			const FShipPartSnapColliderDefinition& ShipPartSnapColliderDefinition = FindShipPartSnapColliderDefinition(ShipPartSnapType, SnapId);

			bool bEnableSnapCollider = IsSnapColliderConnectedByEdge(TileCoords, ShipPartSnapColliderDefinition)
				&& !IsSnapColliderBlocked(TileCoords, ShipPartSnapColliderDefinition);

			ShipPartsOneSnapType.SpawnableShipFoundationSnapColliders[SnapId] = bEnableSnapCollider;

		}
	}
}

void AShip::UpdateHasEdgesAtTile(FIntVector TileCoords)
{
	FShipTile& ShipTile = FindOrAddShipTile(TileCoords);
	for (int EdgeId = 0; EdgeId < EDGE_TYPES_COUNT; ++EdgeId)
	{
		ShipTile.HasEdges[EdgeId] = false;
	}

	for (int X = TileCoords.X - 1; X <= TileCoords.X + 1; ++X)
	{
		for (int Y = TileCoords.Y - 1; Y <= TileCoords.Y + 1; ++Y)
		{
			for (int Z = TileCoords.Z - 1; Z <= TileCoords.Z + 1; ++Z)
			{
				FIntVector NearbyTileCoords = FIntVector(X, Y, Z);
				const FShipTile& NearbyShipTile = FindOrAddShipTile(NearbyTileCoords);

				for (int SnapType = 0; SnapType < SHIP_PART_SNAP_TYPES_COUNT; ++SnapType)
				{
					const FShipPartsOneSnapType& ShipPartsOneSnapType = NearbyShipTile.ShipParts[SnapType];
					for (const FShipFoundationPart& ShipFoundationPart : ShipPartsOneSnapType.ShipFoundationParts)
					{
						const FShipPartSnapColliderDefinition& ShipPartSnapColliderData = FindShipPartSnapColliderDefinition(SnapType, ShipFoundationPart.TileSnapId);
						for (const FSnapColliderEdge& SnapColliderEdge : ShipPartSnapColliderData.Edges)
						{
							if (SnapColliderEdge.TileOffset != TileCoords - NearbyTileCoords)
							{
								continue;
							}
							// we have found edge that is in our tile
							ShipTile.HasEdges[static_cast<int>(SnapColliderEdge.EdgeType)] = true;
						}
					}
				}
			}
		}
	}
}

FShipTile& AShip::FindOrAddShipTile(FIntVector TileCoords)
{
	if (FShipTile* ShipTile = ShipTiles.Find(TileCoords))
	{
		return *ShipTile;
	}
	FShipTile& ShipTile = ShipTiles.Emplace(TileCoords, FShipTile{});

	for (int EdgeIndex = 0; EdgeIndex < EDGE_TYPES_COUNT; ++EdgeIndex)
	{
		ShipTile.HasEdges[EdgeIndex] = false;
	}

	for (int ShipPartSnapType = 0; ShipPartSnapType < SHIP_PART_SNAP_TYPES_COUNT; ++ShipPartSnapType)
	{
		ShipTile.ShipParts[ShipPartSnapType].SpawnableShipFoundationSnapColliders.SetNum(AllSnapDefinitionsArray[ShipPartSnapType].ShipPartSnapColliderDefinitions.Num());
		ShipTile.ShipParts[ShipPartSnapType].SpawnedShipFoundationSnapColliders.SetNum(AllSnapDefinitionsArray[ShipPartSnapType].ShipPartSnapColliderDefinitions.Num());
	}

	return ShipTile;
}

UStaticMeshComponent* AShip::AddSnapCollider(FShipPartSnapHandle ShipPartSnapHandle)
{
	FName Name = FName(FString("ShipSnapColliderAt:") + ShipPartSnapHandle.TileCoords.ToString() + FString("_SnapType:") + FString::FromInt(static_cast<int>(ShipPartSnapHandle.ShipPartSnapType)) + FString("_SnapId:") + FString::FromInt(ShipPartSnapHandle.SnapId));
	UStaticMeshComponent* ShipPartSnapCollider = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), Name);
	
	check(ShipPartSnapCollider != nullptr);
	ShipPartSnapCollider->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	ShipPartSnapCollider->RegisterComponent();
	AddInstanceComponent(ShipPartSnapCollider);
	ShipPartSnapCollider->SetMobility(EComponentMobility::Movable);
	ShipPartSnapCollider->SetHiddenInGame(true);
	ShipPartSnapCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ShipPartSnapCollider->SetCollisionProfileName("ShipPartSnap");
	ShipPartSnapCollider->SetGenerateOverlapEvents(false);

	const FShipPartSnapColliderDefinition& ShipPartSnapColliderDefinition = FindShipPartSnapColliderDefinition(ShipPartSnapHandle.ShipPartSnapType, ShipPartSnapHandle.SnapId);
	
	ShipPartSnapCollider->SetStaticMesh(ShipPartSnapColliderDefinition.ColliderMesh);
	FVector RelativeSnapColliderLocation = GetRelativeTileLocation(ShipPartSnapHandle.TileCoords) + ShipPartSnapColliderDefinition.ColliderOffsetFromTile;
	ShipPartSnapCollider->SetRelativeLocation(RelativeSnapColliderLocation);
	ShipPartSnapCollider->SetRelativeRotation(ShipPartSnapColliderDefinition.ColliderRelativeRotation);
	ShipPartSnapCollider->SetRelativeScale3D(ShipPartSnapColliderDefinition.ColliderScale);

	SnapColliderToSnapHandle.Emplace(ShipPartSnapCollider, ShipPartSnapHandle);

	return ShipPartSnapCollider;
}

void AShip::DestroySnapCollider(UStaticMeshComponent* ColliderMeshComponent)
{
	SnapColliderToSnapHandle.Remove(ColliderMeshComponent);
	ColliderMeshComponent->DestroyComponent();
	RemoveInstanceComponent(ColliderMeshComponent);
}

const FShipPartSnapColliderDefinition& AShip::FindShipPartSnapColliderDefinition(int ShipPartSnapType, int SnapId) const
{
	check(ShipPartSnapType < SHIP_PART_SNAP_TYPES_COUNT);

	const FSnapDefinitionsArray& SnapDefinitionsArray = AllSnapDefinitionsArray[ShipPartSnapType];
	check(SnapId < SnapDefinitionsArray.ShipPartSnapColliderDefinitions.Num());

	return SnapDefinitionsArray.ShipPartSnapColliderDefinitions[SnapId];
}

bool AShip::IsSnapColliderConnectedByEdge(FIntVector TileCoords,
	const FShipPartSnapColliderDefinition& ShipPartSnapColliderDefinition)
{
	for (FSnapColliderEdge SnapColliderEdge : ShipPartSnapColliderDefinition.Edges)
	{
		if (!SnapColliderEdge.bCanUseForSnap)
		{
			continue;
		}
		// this is the tile that the requested edge belongs to
		const FShipTile& EdgeShipTile = FindOrAddShipTile(TileCoords + SnapColliderEdge.TileOffset);
		if (EdgeShipTile.HasEdges[static_cast<int>(SnapColliderEdge.EdgeType)])
		{
			return true;
		}
	}
	return false;
}

bool AShip::IsSnapColliderBlocked(FIntVector TileCoords,
                                  const FShipPartSnapColliderDefinition& ShipPartSnapColliderDefinition)
{
	for (FShipPartSnapHandle BlockedShipPartSnapHandle : ShipPartSnapColliderDefinition.BlockedSnapsRelativeTileCoords)
	{
		const FShipTile& BlockedShipTile = FindOrAddShipTile(TileCoords + BlockedShipPartSnapHandle.TileCoords);
		const auto& BlockedShipFoundationParts = BlockedShipTile.ShipParts[BlockedShipPartSnapHandle.ShipPartSnapType].ShipFoundationParts;
		for (const FShipFoundationPart& BlockedShipFoundationPart : BlockedShipFoundationParts)
		{
			if (BlockedShipFoundationPart.TileSnapId == BlockedShipPartSnapHandle.SnapId)
			{
				return true;
			}
		}
	}
	const FShipTile& ThisShipTile = FindOrAddShipTile(TileCoords);
	const auto& ThisTileShipFoundationParts = ThisShipTile.ShipParts[ShipPartSnapColliderDefinition.SnapType].ShipFoundationParts;
	
	for (const FShipFoundationPart& ThisTileShipFoundationPart : ThisTileShipFoundationParts)
	{
		if (ThisTileShipFoundationPart.TileSnapId == ShipPartSnapColliderDefinition.SnapId)
		{
			return true;
		}
	}
	
	return false;
}

void AShip::FillAllSnapDefinitionsArray()
{
	for (const auto& SnapDefinitions : AllSnapDefinitionsMap)
	{
		int ShipPartSnapType = static_cast<int>(SnapDefinitions.Key);
		check(ShipPartSnapType < SHIP_PART_SNAP_TYPES_COUNT);

		AllSnapDefinitionsArray[ShipPartSnapType] = SnapDefinitions.Value;
		for (int SnapId = 0; SnapId < SnapDefinitions.Value.ShipPartSnapColliderDefinitions.Num(); ++SnapId)
		{
			AllSnapDefinitionsArray[ShipPartSnapType].ShipPartSnapColliderDefinitions[SnapId].SnapId = SnapId;
			AllSnapDefinitionsArray[ShipPartSnapType].ShipPartSnapColliderDefinitions[SnapId].SnapType = ShipPartSnapType;
		}
	}
}

void AShip::ThrottleValueChanged(float ThrottleValue)
{
	for (AShipEngine* ShipEngine : ShipEngines)
	{
		ShipEngine->SetShipThrottleValue(ThrottleValue);
	}
}

void AShip::UpdateSnapColliderComponentsOfType(FIntVector TileCoords, int ShipPartSnapType)
{
	FShipTile& ShipTile = FindOrAddShipTile(TileCoords);
	
	FShipPartsOneSnapType& ShipPartsOneSnapType = ShipTile.ShipParts[ShipPartSnapType];
	for (int SnapId = 0; SnapId < ShipPartsOneSnapType.SpawnableShipFoundationSnapColliders.Num(); ++SnapId)
	{
		const FShipPartSnapColliderDefinition& ShipPartSnapColliderDefinition = FindShipPartSnapColliderDefinition(ShipPartSnapType, SnapId);

		if (ShipPartsOneSnapType.SpawnableShipFoundationSnapColliders[SnapId])
		{
			if (ShipPartsOneSnapType.SpawnedShipFoundationSnapColliders[SnapId] == nullptr)
			{
				ShipPartsOneSnapType.SpawnedShipFoundationSnapColliders[SnapId] = AddSnapCollider(FShipPartSnapHandle {ShipPartSnapType, TileCoords, SnapId});
			}
		} else
		{
			if (ShipPartsOneSnapType.SpawnedShipFoundationSnapColliders[SnapId] != nullptr)
			{
				DestroySnapCollider(ShipPartsOneSnapType.SpawnedShipFoundationSnapColliders[SnapId]);
				ShipPartsOneSnapType.SpawnedShipFoundationSnapColliders[SnapId] = nullptr;
			}
		}
	}
}

void AShip::DestroySnapColliderComponentsOfType(FIntVector TileCoords, int ShipPartSnapType)
{
	FShipTile& ShipTile = FindOrAddShipTile(TileCoords);
	
	FShipPartsOneSnapType& ShipPartsOneSnapType = ShipTile.ShipParts[ShipPartSnapType];
	for (int SnapId = 0; SnapId < ShipPartsOneSnapType.SpawnableShipFoundationSnapColliders.Num(); ++SnapId)
	{
		if (ShipPartsOneSnapType.SpawnedShipFoundationSnapColliders[SnapId] != nullptr)
		{
			DestroySnapCollider(ShipPartsOneSnapType.SpawnedShipFoundationSnapColliders[SnapId]);
			ShipPartsOneSnapType.SpawnedShipFoundationSnapColliders[SnapId] = nullptr;
		}
	}
}

void AShip::TileFoundationsChanged(FIntVector TileCoords)
{
	for (int X = TileCoords.X - 1; X <= TileCoords.X + 1; ++X)
	{
		for (int Y = TileCoords.Y - 1; Y <= TileCoords.Y + 1; ++Y)
		{
			for (int Z = TileCoords.Z - 1; Z <= TileCoords.Z + 1; ++Z)
			{
				UpdateHasEdgesAtTile(FIntVector(X, Y, Z));
			}
		}
	}

	for (int X = TileCoords.X - 1; X <= TileCoords.X + 1; ++X)
	{
		for (int Y = TileCoords.Y - 1; Y <= TileCoords.Y + 1; ++Y)
		{
			for (int Z = TileCoords.Z - 1; Z <= TileCoords.Z + 1; ++Z)
			{
				UpdateSnapCollidersAtTile(FIntVector(X, Y, Z));
			}
		}
	}

	if (bHasSpawnedSnapCollidersType)
	{
		EnableSnapsOfType(SpawnedSnapCollidersType, PlayerTileCoordsForSnap, CurrentSnapTypeMaxRange);
	}

	UpdateTileBounds();
	UpdateRoofsAtTile(TileCoords.X, TileCoords.Y);
}

void AShip::UpdateTileBounds()
{
	if (ShipTiles.Num() == 0)
	{
		return;
	}
	
	BottomLeftTileBound = ShipTiles.begin()->Key;
	TopRightTileBound = ShipTiles.begin()->Key;
	for (const auto& ShipTile : ShipTiles)
	{
		BottomLeftTileBound.X = FMath::Min(BottomLeftTileBound.X, ShipTile.Key.X);
		BottomLeftTileBound.Y = FMath::Min(BottomLeftTileBound.Y, ShipTile.Key.Y);
		BottomLeftTileBound.Z = FMath::Min(BottomLeftTileBound.Z, ShipTile.Key.Z);
		
		TopRightTileBound.X = FMath::Max(TopRightTileBound.X, ShipTile.Key.X);
		TopRightTileBound.Y = FMath::Max(TopRightTileBound.Y, ShipTile.Key.Y);
		TopRightTileBound.Z = FMath::Max(TopRightTileBound.Z, ShipTile.Key.Z);
	}
}

void AShip::UpdateRoofsAtTile(int XCoord, int YCoord)
{
	bool bFoundRoof = false;
	
	for (int ZIter = TopRightTileBound.Z; ZIter >= BottomLeftTileBound.Z; --ZIter)
	{
		FShipTile* ShipTile = ShipTiles.Find(FIntVector(XCoord, YCoord, ZIter));
		if (ShipTile == nullptr)
		{
			continue;
		}
		ShipTile->bTileHasRoof = bFoundRoof;
		
		if (!bFoundRoof)
		{
			if (ShipTile->ShipParts[static_cast<int>(EShipPartSnapTypes::FLOOR)].ShipFoundationParts.Num() == 1)
			{
				bFoundRoof = true;
			} else
			{
				if (ShipTile->ShipParts[static_cast<int>(EShipPartSnapTypes::TRIANGLE_FLOOR)].ShipFoundationParts.Num() == 2)
				{
					bFoundRoof = true;
				}
			}
		}
	}
}

void AShip::EnableSnapsOfType(EShipPartSnapTypes ShipPartSnapType, FVector PlayerLocation, float MaxRange)
{
	const FIntVector CurrentPlayerTileCoords = GetTileCoordsFromWorldLocation(PlayerLocation);
	EnableSnapsOfType(static_cast<int>(ShipPartSnapType), CurrentPlayerTileCoords, MaxRange);
}

void AShip::EnableSnapsOfType(int ShipPartSnapType, FIntVector CurrentPlayerTileCoords, float MaxRange)
{
	const int MaxTileOffset = FMath::CeilToInt(MaxRange / TILE_LENGTH);
	
	if (bHasSpawnedSnapCollidersType)
	{
		check(ShipPartSnapType == SpawnedSnapCollidersType);

		// iterate old spawned colliders and check if some of them should be destroyed
		for (int X = PlayerTileCoordsForSnap.X - MaxTileOffset; X <= PlayerTileCoordsForSnap.X + MaxTileOffset; ++X)
		{
			for (int Y = PlayerTileCoordsForSnap.Y - MaxTileOffset; Y <= PlayerTileCoordsForSnap.Y + MaxTileOffset; ++Y)
			{
				for (int Z = PlayerTileCoordsForSnap.Z - MaxTileOffset; Z <= PlayerTileCoordsForSnap.Z + MaxTileOffset; ++Z)
				{
					const FIntVector IterTileCoords {X, Y, Z};
					if (!IsIntVectorInBounds(CurrentPlayerTileCoords - MaxTileOffset, CurrentPlayerTileCoords + MaxTileOffset, IterTileCoords))
					{
						DestroySnapColliderComponentsOfType(IterTileCoords, ShipPartSnapType);
					}
				}
			}
		}
	} else
	{
		bHasSpawnedSnapCollidersType = true;
		SpawnedSnapCollidersType = ShipPartSnapType;
		CurrentSnapTypeMaxRange = MaxRange;
	}
	
	for (int X = CurrentPlayerTileCoords.X - MaxTileOffset; X <= CurrentPlayerTileCoords.X + MaxTileOffset; ++X)
	{
		for (int Y = CurrentPlayerTileCoords.Y - MaxTileOffset; Y <= CurrentPlayerTileCoords.Y + MaxTileOffset; ++Y)
		{
			for (int Z = CurrentPlayerTileCoords.Z - MaxTileOffset; Z <= CurrentPlayerTileCoords.Z + MaxTileOffset; ++Z)
			{
				UpdateSnapColliderComponentsOfType(FIntVector(X, Y, Z), ShipPartSnapType);
			}
		}
	}

	PlayerTileCoordsForSnap = CurrentPlayerTileCoords;
}

void AShip::DisableSnapsOfType()
{
	const int MaxTileOffset = FMath::CeilToInt(CurrentSnapTypeMaxRange / TILE_LENGTH);
	for (int X = PlayerTileCoordsForSnap.X - MaxTileOffset; X <= PlayerTileCoordsForSnap.X + MaxTileOffset; ++X)
	{
		for (int Y = PlayerTileCoordsForSnap.Y - MaxTileOffset; Y <= PlayerTileCoordsForSnap.Y + MaxTileOffset; ++Y)
		{
			for (int Z = PlayerTileCoordsForSnap.Z - MaxTileOffset; Z <= PlayerTileCoordsForSnap.Z + MaxTileOffset; ++Z)
			{
				DestroySnapColliderComponentsOfType(FIntVector(X, Y, Z), SpawnedSnapCollidersType);
			}
		}
	}
	bHasSpawnedSnapCollidersType = false;
}

bool AShip::GetShipPartHandleFromHit(const FHitResult& HitResult, FShipPartHandle& ShipPartHandle)
{
	if (HitResult.GetActor() != this)
	{
		// it can only be ship part actor
		AShipPart* HitShipPart = Cast<AShipPart>(HitResult.GetActor());
		if (HitShipPart == nullptr)
		{
			return false;
		}
		if (ShipPartChildActors.Contains(HitShipPart))
		{
			ShipPartHandle.SetSubtype<FShipActorPartHandle>(FShipActorPartHandle{HitShipPart});
			return true;
		}
		return false;
	} else
	{
		UStaticMeshComponent* HitStaticMesh = Cast<UStaticMeshComponent>(HitResult.GetComponent());
		if (HitStaticMesh == nullptr)
		{
			return false;
		}
		// we hit this ship
		if (FoundationMeshToSnapHandle.Contains(HitStaticMesh))
		{
			ShipPartHandle.SetSubtype<FShipFoundationPartHandle>(FShipFoundationPartHandle{HitStaticMesh});
			return true;
		}
		return false;
	}
}

void AShip::DeconstructShipPart(FShipPartHandle ShipPartHandle)
{
	if (ShipPartHandle.HasSubtype<FShipActorPartHandle>())
	{
		AShipPart* ShipPartActor = ShipPartHandle.GetSubtype<FShipActorPartHandle>().ShipPartActor;
		check(ShipPartActor != nullptr);
		check(ShipPartChildActors.Contains(ShipPartActor));
		ShipPartChildActors.Remove(ShipPartActor);
		
		if (AShipEngine* DeconstructedShipEngine = Cast<AShipEngine>(ShipPartActor))
		{
			ShipEngines.Remove(DeconstructedShipEngine);
		}

		int ShipPartChildActorComponentIndex = -1;
		for (int i = 0; i < ShipPartChildActorComponents.Num(); ++i)
		{
			if (ShipPartChildActorComponents[i]->GetChildActor() == ShipPartActor)
			{
				ShipPartChildActorComponentIndex = i;
				break;
			}
		}
		check(ShipPartChildActorComponentIndex != -1);
		ShipPartActor->Destroy();
		
		RemoveInstanceComponent(ShipPartChildActorComponents[ShipPartChildActorComponentIndex]);
		ShipPartChildActorComponents[ShipPartChildActorComponentIndex]->DestroyComponent();
		ShipPartChildActorComponents.RemoveAtSwap(ShipPartChildActorComponentIndex);
		return;
	}

	if (ShipPartHandle.HasSubtype<FShipFoundationPartHandle>())
	{
		UStaticMeshComponent* ShipFoundationPartStaticMesh = ShipPartHandle.GetSubtype<FShipFoundationPartHandle>().ShipFoundationPartStaticMesh;
		FShipPartSnapHandle* ShipPartSnapHandle = FoundationMeshToSnapHandle.Find(ShipFoundationPartStaticMesh);
		check(ShipPartSnapHandle != nullptr);

		FoundationMeshToSnapHandle.Remove(ShipFoundationPartStaticMesh);

		FShipTile& ShipTile = FindOrAddShipTile(ShipPartSnapHandle->TileCoords);
		auto& ShipFoundationParts = ShipTile.ShipParts[ShipPartSnapHandle->ShipPartSnapType].ShipFoundationParts;

		int IndexToRemove = -1;
		for (int i = 0; i < ShipFoundationParts.Num(); ++i)
		{
			if (ShipFoundationParts[i].TileSnapId == ShipPartSnapHandle->SnapId)
			{
				IndexToRemove = i;
				break;
			}
		}
		check(IndexToRemove != -1);
		ShipFoundationParts.RemoveAtSwap(IndexToRemove);

		ShipFoundationPartStaticMesh->DestroyComponent();
		RemoveInstanceComponent(ShipFoundationPartStaticMesh);

		TileFoundationsChanged(ShipPartSnapHandle->TileCoords);
	}
}

UPrimitiveComponent* AShip::GetMovingPrimitiveComponent()
{
	return BoundaryBoxComponent;
}

bool AShip::IsPointInside(FVector Location)
{
	FIntVector TileCoords = GetTileCoordsFromWorldLocation(Location);
	FShipTile* ShipTile = ShipTiles.Find(TileCoords);
	if (ShipTile == nullptr)
	{
		return false;
	}
	return ShipTile->bTileHasRoof;
}

float AShip::ConsumeEnginePower(float ThrottleAmount, float DeltaTime)
{
	float RequiredPower = RequiredPowerForFullThrottlePerSecond * ThrottleAmount * DeltaTime;

	if (RequiredPower <= 0.0f)
	{
		return 0.0f;
	}

	TArray<TPair<float, IPowerProducer*>> SortedPowerProducers;
	for (IPowerProducer* PowerProducer : PowerProducers)
	{
		float AvailablePower = PowerProducer->GetAvailablePower();
		if (AvailablePower > 0.0f)
		{
			SortedPowerProducers.Emplace(AvailablePower, PowerProducer);
		}
	}
	SortedPowerProducers.Sort();

	float AvailablePower = 0.0f;

	for (int i = 0; i < SortedPowerProducers.Num(); ++i)
	{
		int PowerProducersLeft = SortedPowerProducers.Num() - i;
		float ThisRoundPowerPerProducer = i > 0 ? (SortedPowerProducers[i].Key - SortedPowerProducers[i - 1].Key) : SortedPowerProducers[i].Key;
		float ThisRoundPower = PowerProducersLeft * ThisRoundPowerPerProducer;

		if (AvailablePower + ThisRoundPower >= RequiredPower)
		{
			// we have enough power, so take the remaining power out of the last Power Producers
			float PowerToTakeThisRound = RequiredPower - AvailablePower;
			float PowerPerFullProducer = i > 0 ? SortedPowerProducers[i - 1].Key : 0.0f + PowerToTakeThisRound / PowerProducersLeft;

			for (int FullPowerProducer = i; FullPowerProducer < SortedPowerProducers.Num(); ++FullPowerProducer)
			{
				SortedPowerProducers[FullPowerProducer].Value->TakePower(PowerPerFullProducer);
			}
			ThrottleValueChanged(ThrottleAmount);
			return ThrottleAmount;
		}
		
		// we still don't have enough power, so take all power from the i-th PowerProducer and continue
		SortedPowerProducers[i].Value->TakePower(SortedPowerProducers[i].Key);

		AvailablePower += ThisRoundPower;
	}

	float RequiredPowerForMaxThrottle = RequiredPowerForFullThrottlePerSecond * DeltaTime;
	const float NewThrottleAmount = AvailablePower / RequiredPowerForMaxThrottle;
	ThrottleValueChanged(NewThrottleAmount);
	return NewThrottleAmount;
}

AShip* AShip::FindParentShip(AActor* Actor)
{
	while (Actor != nullptr)
	{
		if (AShip* Ship = Cast<AShip>(Actor))
		{
			return Ship;
		}
		Actor = Actor->GetAttachParentActor();
	}
	return nullptr;
}

FShipPartSnapHitResult AShip::EvaluateSnapHit(const FHitResult& HitResult)
{
	FShipPartSnapHandle* HitShipPartSnapHandle = SnapColliderToSnapHandle.Find(HitResult.GetComponent());
	
	if (HitShipPartSnapHandle == nullptr)
	{
		// this can happen for various reasons, such as collider we just deleted is still hittable
		return FShipPartSnapHitResult {false};
	}

	const FShipPartSnapColliderDefinition& ShipPartSnapColliderDefinition = FindShipPartSnapColliderDefinition(HitShipPartSnapHandle->ShipPartSnapType, HitShipPartSnapHandle->SnapId);

	FVector RelativeSnapLocation = GetRelativeTileLocation(HitShipPartSnapHandle->TileCoords) + ShipPartSnapColliderDefinition.SnapPointOffsetFromTile;
	return FShipPartSnapHitResult {true, RelativeSnapLocation, ShipPartSnapColliderDefinition.SnapPointRelativeRotation, ShipPartSnapColliderDefinition.SnapPointScale, *HitShipPartSnapHandle};
}

void AShip::ShipHitWithBullet(UPrimitiveComponent* HitComponent, const FTransform& DestructionCapsuleTransform,
	float CapsuleHeight, float CapsuleRadius)
{
}

// Called every frame
void AShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

UShipFloatingMovement* AShip::GetShipFloatingMovement()
{
    return ShipFloatingMovement;
}

void AShip::AddSteering(float Direction)
{
	ShipFloatingMovement->AddInputVector(FVector(0.0f, 0.0f, Direction));
}

void AShip::AddThrottle(float Direction)
{
	ShipFloatingMovement->AddInputVector(FVector(Direction, 0.0f, 0.0f));
}

void AShip::AddPitch(float Direction)
{
	ShipFloatingMovement->AddInputVector(FVector(0.0f, Direction, 0.0f));
}

void AShip::AddShipActorPart(UShipPartDataAsset* ShipPartDataAsset, FVector Location, FRotator Rotation)
{
	check(ShipPartDataAsset != nullptr);
	check(ShipPartDataAsset->ShipPartActorClass != nullptr);
	
	FName Name = FName(FString("ShipActorPart_") + FString::FromInt(ShipPartChildActors.Num()) + FString("_") + ShipPartDataAsset->ShipPartActorClass->GetName());
	UChildActorComponent* ShipPartChildActorComponent = NewObject<UChildActorComponent>(this, UChildActorComponent::StaticClass(), Name);
	check(ShipPartChildActorComponent != nullptr);
	ShipPartChildActorComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	ShipPartChildActorComponent->RegisterComponent();
	AddInstanceComponent(ShipPartChildActorComponent);
	
	ShipPartChildActorComponent->SetWorldLocation(Location);
	ShipPartChildActorComponent->SetWorldRotation(Rotation);
	ShipPartChildActorComponent->SetChildActorClass(ShipPartDataAsset->ShipPartActorClass);
	ShipPartChildActorComponent->CreateChildActor();
	ShipPartChildActorComponents.Add(ShipPartChildActorComponent);

	AShipPart* ShipPart = Cast<AShipPart>(ShipPartChildActorComponent->GetChildActor());
	check(ShipPart != nullptr);
	ShipPartChildActors.Add(ShipPart);

	if (AShipEngine* NewShipEngine = Cast<AShipEngine>(ShipPartChildActorComponent->GetChildActor()))
	{
		ShipEngines.Add(NewShipEngine);
	}

	CalculateEngineStats();
}

bool AShip::AddFoundationShipPart(const FSerializedShipFoundationPartData& ShipFoundationPartData)
{
	int SnapType = static_cast<int>(ShipFoundationPartData.ShipPartDataAsset->ShipPartSnapType);
	check(SnapType < SHIP_PART_SNAP_TYPES_COUNT);

	const FShipTile& ShipTile = FindOrAddShipTile(ShipFoundationPartData.ShipTileCoords);
	const FShipPartsOneSnapType& ShipPartsOneSnap = ShipTile.ShipParts[SnapType];

	check(ShipFoundationPartData.TileSnapId < ShipPartsOneSnap.SpawnableShipFoundationSnapColliders.Num());

	// checking if snap collider is null or not is equivalent to evaluating ship data
	if (!ShipPartsOneSnap.SpawnableShipFoundationSnapColliders[ShipFoundationPartData.TileSnapId])
	{
		return false;
	}
	SpawnShipFoundationPart(ShipFoundationPartData);
	return true;
}

void AShip::AddPowerProducer(IPowerProducer* PowerProducer)
{
	PowerProducers.Add(PowerProducer);
}

void AShip::RemovePowerProducer(IPowerProducer* PowerProducer)
{
	PowerProducers.Remove(PowerProducer);
}

