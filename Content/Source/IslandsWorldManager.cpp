// Fill out your copyright notice in the Description page of Project Settings.


#include "IslandsWorldManager.h"

#include "BiomeDataAsset.h"
#include "CollisionTraceChannels.h"
#include "EnvironmentPropDataAsset.h"
#include "EnvironmentPropsInstancer.h"
#include "MeshColliderOverlap.h"
#include "MyGameInstance.h"
#include "VoxelIslandGenerator.h"
#include "VoxelIslandWorld.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Voronoi/Voronoi.h"
#include "VoxelData/VoxelData.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelTools/VoxelSurfaceTools.h"

AIslandsWorldManager* AIslandsWorldManager::Instance = nullptr;


// Sets default values
AIslandsWorldManager::AIslandsWorldManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AIslandsWorldManager::BeginPlay()
{
	Super::BeginPlay();

	Instance = this;

	NumOfEnvironmentPartitionsToLoadAroundPlayer = FMath::CeilToInt(MinDistToLoadEnvironmentProp / EnvironmentPartitionCellSize); 
	
	if (UKismetSystemLibrary::IsServer(GetWorld()))
	{
		GenerateWorldOnServer();
	} else
	{
		GenerateWorldFromSeed();
	}

	GetWorldTimerManager().SetTimer(SpawnIslandsEnvironmentsHandle, this, &AIslandsWorldManager::SpawnIslandsEnvironments, 1.0f, true);
	GetWorldTimerManager().SetTimer(LoadIslandEnvironmentPartitionTimerHandle, this, &AIslandsWorldManager::LoadQueuedIslandEnvironmentPartitions, 0.1f, true);
}

void AIslandsWorldManager::GenerateWorldFromSeed()
{
	TMap<EVoxelTaskType, int32> PriorityCategoriesOverrides;
	TMap<EVoxelTaskType, int32> PriorityOffsetsOverrides;
	UVoxelBlueprintLibrary::CreateGlobalVoxelThreadPool(PriorityCategoriesOverrides, PriorityOffsetsOverrides, 8);
	WorldRandomStream.Initialize(WorldSeed);
	AddHierarchicalInstancedMeshComponents();
	GenerateBiomeInstancesOrigins();
	SpawnIslands();
}

void AIslandsWorldManager::GenerateWorldOnServer()
{
	UMyGameInstance* MyGameInstance = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	check(MyGameInstance);
	if (MyGameInstance->IsSeedSet())
	{
		WorldSeed = MyGameInstance->GetSeed();
	} else
	{
		WorldSeed = FMath::RandRange(0, TNumericLimits<int32>::Max() - 1);
	}
	
	GenerateWorldFromSeed();
}

void AIslandsWorldManager::GenerateBiomeInstancesOrigins()
{
	check(TemperatureByYCoordCurve != nullptr);

	for (int BiomeInstanceId = 0; BiomeInstanceId < NumberOfBiomeInstances; ++BiomeInstanceId)
	{
		float X = WorldRandomStream.FRandRange(-FillableWorldSize / 2, FillableWorldSize / 2);
		float Y = WorldRandomStream.FRandRange(-FillableWorldSize / 2, FillableWorldSize / 2);

		float Temperature = TemperatureByYCoordCurve->GetFloatValue(Y / FillableWorldSize);

		TArray<float> BiomeCumStrengths;

		float StrengthsSum = 0.0f;
		for (const UBiomeDataAsset* Biome : AllBiomeDataAssets)
		{
			float Strength = Biome->BiomeStrengthByTemperatureCurve->GetFloatValue(Temperature);
			StrengthsSum += Strength;
			BiomeCumStrengths.Add(StrengthsSum);
		}

		float RandStrength = WorldRandomStream.FRandRange(0.0f, StrengthsSum);
		int ChosenBiome = Algo::LowerBound(BiomeCumStrengths, RandStrength);
		BiomeInstances.Add(FIslandBiomeInstance {AllBiomeDataAssets[ChosenBiome], FVector2D(X, Y)} );
		BiomeInstances.Last().BiomeInstanceId = BiomeInstanceId;
		UE_LOG(LogTemp, Warning, TEXT("spawning biome %s with temperature %f.  at: %s"), *AllBiomeDataAssets[ChosenBiome]->GetName(), Temperature, *FVector2D(X, Y).ToString());
	}
	check(BiomeInstances.Num() > 0);
}

void AIslandsWorldManager::SpawnIslands()
{
	TArray<FVector> VoronoiSites;
	for (const FIslandBiomeInstance& BiomeInstance : BiomeInstances)
	{
		VoronoiSites.Add(FVector(BiomeInstance.WorldLocation, 0.0f));
	}
	FVector Min = FVector(-FillableWorldSize / 2, -FillableWorldSize / 2, 0.0f);
	FVector Max = FVector(FillableWorldSize / 2, FillableWorldSize / 2, 0.0f);
	FBox Box = FBox(Min, Max);
	
	TArray<FVoronoiCellInfo> BiomeInstancesVoronoiCells;
	FVoronoiDiagram BiomeInstancesVoronoiDiagram (VoronoiSites, Box, 0.0f);
	{
		BiomeInstancesVoronoiDiagram.ComputeAllCells(BiomeInstancesVoronoiCells);
	}
	
	MaxIslandRadiusInCm = 0.0f;

	for (const UBiomeDataAsset* BiomeDataAsset : AllBiomeDataAssets)
	{
		// TODO(Roman): this uses Voxel Size constant
		MaxIslandRadiusInCm = FMath::Max(MaxIslandRadiusInCm, BiomeDataAsset->MaxIslandRadiusInVoxels * 100.0f);
	}
	
	for (int i = 0; i < NumberOfIslands; ++i)
	{
		float IslandOriginInCmX = WorldRandomStream.FRandRange(-FillableWorldSize / 2 + MaxIslandRadiusInCm, FillableWorldSize / 2 - MaxIslandRadiusInCm);
		float IslandOriginInCmY = WorldRandomStream.FRandRange(-FillableWorldSize / 2 + MaxIslandRadiusInCm, FillableWorldSize / 2 - MaxIslandRadiusInCm);

		if (FVector2D::Distance(FVector2D(IslandOriginInCmX, IslandOriginInCmY), FVector2D::ZeroVector) < MaxIslandRadiusInCm * 2.0f)
		{
			continue;
		}
		
		int ClosestBiome = BiomeInstancesVoronoiDiagram.FindCell(FVector(IslandOriginInCmX, IslandOriginInCmY, 0));

		const FIslandBiomeInstance& IslandBiomeInstance = BiomeInstances[ClosestBiome];
		int IslandRadiusInVoxels = WorldRandomStream.RandRange(IslandBiomeInstance.BiomeRef->MinIslandRadiusInVoxels, IslandBiomeInstance.BiomeRef->MaxIslandRadiusInVoxels);
		float IslandRadiusInCm = IslandRadiusInVoxels * 100.0f;
		
		//FVoxelIslandGeneratorInstance* VoxelIslandGeneratorInstance = static_cast<FVoxelIslandGeneratorInstance*>(&IslandVoxelWorld->Generator.GetInstance(true).Get());
		/*FVoxelIslandGeneratorInstanceBase* VoxelIslandGeneratorInstance = dynamic_cast<FVoxelIslandGeneratorInstanceBase*>(&IslandVoxelWorld->GetData().Generator.Get());
		check(VoxelIslandGeneratorInstance != nullptr);
		VoxelIslandGeneratorInstance->IslandRadius = IslandRadius;
		VoxelIslandGeneratorInstance->MaterialIndex = IslandBiomeInstance.BiomeRef->MaterialIndex;
		VoxelIslandGeneratorInstance->SetSeed(WorldRandomStream.RandRange(0, TNumericLimits<int32>::Max() - 1));*/

		TPair<int32, int32> IslandPartitionCoords = GetIslandPartitionCoords(FVector2D(IslandOriginInCmX, IslandOriginInCmY));

		// check for overlap with other islands by iterating nearby cells
		// this is because island partition cells size is equal to MaxIslandRadiusInCm, so 2 islands
		// can never overlap if they are in farther than 2 away
		int32 NumOfPartitionCellsToCheck = 2;

		bool PositionOk = true;

		for (int32 CheckIslandPartitionCoordY = IslandPartitionCoords.Value - NumOfPartitionCellsToCheck; CheckIslandPartitionCoordY <= IslandPartitionCoords.Value + NumOfPartitionCellsToCheck; ++CheckIslandPartitionCoordY)
		{
			for (int32 CheckIslandPartitionCoordX = IslandPartitionCoords.Key - NumOfPartitionCellsToCheck; CheckIslandPartitionCoordX <= IslandPartitionCoords.Key + NumOfPartitionCellsToCheck; ++CheckIslandPartitionCoordX)
			{
				TArray<AVoxelIslandWorld*>* PartitionIslands = VoxelIslandsPartitionCells.Find(TPair<int32, int32>(CheckIslandPartitionCoordX, CheckIslandPartitionCoordY));
				if (PartitionIslands == nullptr)
				{
					continue;
				}
				for (AVoxelIslandWorld* NearbyIsland : *PartitionIslands)
				{
					float IslandsDistance = FVector2D::Distance(FVector2D(IslandOriginInCmX, IslandOriginInCmY), FVector2D(NearbyIsland->GetActorLocation()));
					if (IslandsDistance <= IslandRadiusInCm + NearbyIsland->IslandRadiusInVoxels * 100.0f)
					{
						PositionOk = false;
						break;
					}
				}
				if (!PositionOk)
				{
					break;
				}
			}
			if (!PositionOk)
			{
				break;
			}
		}

		if (!PositionOk)
		{
			continue;
		}

		AVoxelIslandWorld* IslandVoxelWorld = GetWorld()->SpawnActor<AVoxelIslandWorld>(IslandBiomeInstance.BiomeRef->VoxelWorldClass, FVector(IslandOriginInCmX, IslandOriginInCmY, 0), FRotator::ZeroRotator);
		check(IslandVoxelWorld != nullptr);
		IslandVoxelWorld->IslandRadiusInVoxels = IslandRadiusInVoxels;
		IslandVoxelWorld->IslandBiomeDataAsset = IslandBiomeInstance.BiomeRef;
		
		IslandVoxelWorld->SetWorldSize(IslandRadiusInVoxels * 2 + 1);
		IslandVoxelWorld->CreateWorld();
		
		VoxelIslandsPartitionCells.FindOrAdd(IslandPartitionCoords).Add(IslandVoxelWorld);

		// fill environment partitions that island occupies
		TPair<int32, int32> BottomLeftPartition = GetEnvironmentPartitionCoords(FVector2D(IslandOriginInCmX - IslandRadiusInCm, IslandOriginInCmY - IslandRadiusInCm));
		TPair<int32, int32> TopRightPartition = GetEnvironmentPartitionCoords(FVector2D(IslandOriginInCmX + IslandRadiusInCm, IslandOriginInCmY + IslandRadiusInCm));

		for (int PartitionY = BottomLeftPartition.Value; PartitionY <= TopRightPartition.Value; ++PartitionY)
		{
			for (int PartitionX = BottomLeftPartition.Key; PartitionX <= TopRightPartition.Key; ++PartitionX)
			{
				FEnvironmentPartition& EnvironmentPartition = EnvironmentPartitionCells.FindOrAdd(TPair<int32, int32>(PartitionX, PartitionY));
				EnvironmentPartition.BottomLeftLocation = FVector2D(PartitionX, PartitionY) * EnvironmentPartitionCellSize;
				EnvironmentPartition.PresentVoxelWorlds.Add(IslandVoxelWorld);
			}
		} 
	}
}

void AIslandsWorldManager::SpawnIslandsEnvironments()
{
	// TODO(Roman): this will only spawn environment around the server player - need to spawn around all players
	TPair<int32, int32> EnvironmentCellAtPlayer = GetEnvironmentPartitionCoords(FVector2D(UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->GetActorLocation()));
	
	for (int32 PartitionY = EnvironmentCellAtPlayer.Value - NumOfEnvironmentPartitionsToLoadAroundPlayer; PartitionY <= EnvironmentCellAtPlayer.Value + NumOfEnvironmentPartitionsToLoadAroundPlayer; ++PartitionY)
	{
		for (int32 PartitionX = EnvironmentCellAtPlayer.Key - NumOfEnvironmentPartitionsToLoadAroundPlayer; PartitionX <= EnvironmentCellAtPlayer.Key + NumOfEnvironmentPartitionsToLoadAroundPlayer; ++PartitionX)
		{
			TPair<int32, int32> PartitionCoords(PartitionX, PartitionY);
			if (LoadedEnvironmentPartitions.Contains(PartitionCoords))
			{
				continue;
			}
			LoadEnvironmentPartition(PartitionCoords);
		}
	}
}

void AIslandsWorldManager::LoadEnvironmentPartition(TPair<int32, int32> PartitionCoords)
{
	LoadedEnvironmentPartitions.Add(PartitionCoords);
	FEnvironmentPartition* EnvironmentPartition = EnvironmentPartitionCells.Find(PartitionCoords);
	if (EnvironmentPartition == nullptr)
	{
		return;
	}
	
	for (AVoxelIslandWorld* VoxelIslandWorld : EnvironmentPartition->PresentVoxelWorlds)
	{
		IslandEnvironmentPartitionsToLoad.Add(FIslandEnvironmentPartition {VoxelIslandWorld, EnvironmentPartition->BottomLeftLocation});
	}
}

void AIslandsWorldManager::LoadQueuedIslandEnvironmentPartitions()
{
	for (int i = 0; i < IslandEnvironmentPartitionsToLoad.Num(); ++i)
	{
		FIslandEnvironmentPartition& IslandEnvironmentPartitionToLoad = IslandEnvironmentPartitionsToLoad[i]; 
		// TODO(Roman): check if this check is enough, maybe add another check to test if island collisions are loaded
		if (!IslandEnvironmentPartitionToLoad.PresentVoxelWorld->IsLoaded())
		{
			continue;
		}
		
		SpawnIslandPartitionEnvironment(IslandEnvironmentPartitionToLoad);
		IslandEnvironmentPartitionsToLoad.RemoveAtSwap(i);
		break;
	}
}

void AIslandsWorldManager::SpawnIslandPartitionEnvironment(FIslandEnvironmentPartition& IslandEnvironmentPartition)
{
	AVoxelIslandWorld* VoxelIslandWorld = IslandEnvironmentPartition.PresentVoxelWorld;
	UBiomeDataAsset* IslandBiomeDataAsset = VoxelIslandWorld->IslandBiomeDataAsset;
	
	if (IslandBiomeDataAsset->SpawnableEnvironmentProps.Num() == 0)
	{
		return;
	}
	
	AMeshColliderOverlap* CollisionTestActor = GetWorld()->SpawnActor<AMeshColliderOverlap>(AMeshColliderOverlap::StaticClass());
	check(CollisionTestActor != nullptr);

	float TotalEnvironmentPropsFrequencyPer100m2 = 0.0f;

	float CumFrequency = 0.0f;
	TArray<float> CumFrequencies;
	
	for (const FEnvironmentPropSpawnAttributes& PropSpawnAttributes : IslandBiomeDataAsset->SpawnableEnvironmentProps)
	{
		TotalEnvironmentPropsFrequencyPer100m2 += PropSpawnAttributes.EnvironmentPropFrequencyPer100m2;
		CumFrequency += PropSpawnAttributes.EnvironmentPropFrequencyPer100m2;
		CumFrequencies.Add(CumFrequency);
	}

	if (TotalEnvironmentPropsFrequencyPer100m2 <= 0.0f)
	{
		return;
	}

	float TotalAreaInM2 = EnvironmentPartitionCellSize * EnvironmentPartitionCellSize / 10000;
	
	float PropsToSpawnCount = (TotalAreaInM2 / 100.0f) * TotalEnvironmentPropsFrequencyPer100m2;

	for (int PropToSpawnId = 0; PropToSpawnId < PropsToSpawnCount; ++PropToSpawnId)
	{
		float RandFreq = WorldRandomStream.FRandRange(0.0f, TotalEnvironmentPropsFrequencyPer100m2);
		int ChosenEnvironmentPropId = Algo::LowerBound(CumFrequencies, RandFreq);
		check(ChosenEnvironmentPropId >= 0);
		check(ChosenEnvironmentPropId < CumFrequencies.Num());
		const UEnvironmentPropDataAsset* ChosenProp = IslandBiomeDataAsset->SpawnableEnvironmentProps[ChosenEnvironmentPropId].EnvironmentPropDataAsset;
		
		FVector2D RandomPointInPartitionCell = FVector2D(WorldRandomStream.FRandRange(0.0f, EnvironmentPartitionCellSize), WorldRandomStream.FRandRange(0.0f, EnvironmentPartitionCellSize)) + IslandEnvironmentPartition.BottomLeftLocation; 

		FHitResult GroundHit = LineTraceGround(RandomPointInPartitionCell);
				
		if (GroundHit.Actor != VoxelIslandWorld)
		{
			UE_LOG(LogTemp, Warning, TEXT("can't add instance didn't hit anything at %s"), *RandomPointInPartitionCell.ToString());
			continue;
		}

		FBox MeshBoundingBox = ChosenProp->PropStaticMesh->GetBoundingBox();

		FVector2D BotLeftBound = FVector2D(GroundHit.Location) + FVector2D(MeshBoundingBox.Min);
		FVector2D TopLeftBound = FVector2D(GroundHit.Location) + FVector2D(MeshBoundingBox.Min.X, MeshBoundingBox.Max.Y);
		FVector2D BotRightBound = FVector2D(GroundHit.Location) + FVector2D(MeshBoundingBox.Max.X, MeshBoundingBox.Min.Y);
		FVector2D TopRightBound = FVector2D(GroundHit.Location) + FVector2D(MeshBoundingBox.Max);

		FHitResult BotLeftHit = LineTraceGround(BotLeftBound);
		FHitResult TopLeftHit = LineTraceGround(TopLeftBound);
		FHitResult BotRightHit = LineTraceGround(BotRightBound);
		FHitResult TopRightHit = LineTraceGround(TopRightBound);

		if (BotLeftHit.Actor != VoxelIslandWorld
			|| TopLeftHit.Actor != VoxelIslandWorld
			|| BotRightHit.Actor != VoxelIslandWorld
			|| TopRightHit.Actor != VoxelIslandWorld)
		{
			continue;
		}

		TArray<TPair<const FHitResult&, const FHitResult&>> SlopePairs =
		{
			TPair<const FHitResult&, const FHitResult&>{BotLeftHit, TopLeftHit},
			TPair<const FHitResult&, const FHitResult&>{BotLeftHit, BotRightHit},
			TPair<const FHitResult&, const FHitResult&>{TopLeftHit, TopRightHit},
			TPair<const FHitResult&, const FHitResult&>{BotRightHit, TopRightHit},
		};

		bool SlopeAngleOk = true;
		
		for (const auto& TracePoints : SlopePairs)
		{
			float XYDiff = FMath::Abs(TracePoints.Key.Location.X - TracePoints.Value.Location.X)
				+ FMath::Abs(TracePoints.Key.Location.Y - TracePoints.Value.Location.Y);

			float ZDiff = FMath::Abs(TracePoints.Key.Location.Z - TracePoints.Value.Location.Z);

			float Angle = FMath::Abs(FMath::RadiansToDegrees(FMath::Asin(ZDiff / XYDiff)));
			if (Angle > ChosenProp->MaxSlopeAngle || Angle < ChosenProp->MinSlopeAngle)
			{
				SlopeAngleOk = false;
				break;
			}
		}

		if (!SlopeAngleOk)
		{
			continue;
		}

		float MoveAlongNormal = WorldRandomStream.FRandRange(ChosenProp->MoveAlongTerrainNormalMin, ChosenProp->MoveAlongTerrainNormalMax); 
		FVector FinalLocation = GroundHit.Location - GroundHit.Normal * MoveAlongNormal;

		FRotator FinalRotation = FRotator(0, WorldRandomStream.FRandRange(0.0f, 360.0f), 0.0f);

		// handle collisions with already placed environment props
		FActorSpawnParameters ColliderSpawnParameters;
		ColliderSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		TArray<FOverlapResult> Overlaps;
				
		CollisionTestActor->GetStaticMeshComponent()->SetStaticMesh(ChosenProp->PropStaticMesh);
		GetWorld()->ComponentOverlapMulti(Overlaps, CollisionTestActor->GetStaticMeshComponent(), FinalLocation, FinalRotation);
		
		bool InvalidOverlap = false;
		
		for (const FOverlapResult& Overlap : Overlaps)
		{
			if (Overlap.Actor != VoxelIslandWorld)
			{
				InvalidOverlap = true;
				break;
			}
		}

		if (InvalidOverlap)
		{
			continue;
		}
		
		FTransform Transform;
		Transform.SetLocation(FinalLocation);
		Transform.SetRotation(FinalRotation.Quaternion());
		Transform.SetScale3D(FVector::OneVector);

		AEnvironmentPropsInstancer::GetInstance()->AddInstance(
			ChosenProp, Transform);
	}
	
	CollisionTestActor->Destroy();
}

FHitResult AIslandsWorldManager::LineTraceGround(FVector2D Location)
{
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(GroundTrace), true, GetOwner());
	FVector StartTrace = FVector(Location.X, Location.Y, 100000);
	FVector EndTrace = FVector(Location.X, Location.Y, -100000);

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, GROUND_TRACE_CHANNEL, TraceParams);
	return Hit;
}

void AIslandsWorldManager::AddHierarchicalInstancedMeshComponents()
{
	for (const UBiomeDataAsset* Biome : AllBiomeDataAssets)
	{
		for (FEnvironmentPropSpawnAttributes EnvironmentPropSpawnAttributes : Biome->SpawnableEnvironmentProps)
		{
			UE_LOG(LogTemp, Warning, TEXT("register instancee"));
			AEnvironmentPropsInstancer::GetInstance()->RegisterEnvironmentProp(EnvironmentPropSpawnAttributes.EnvironmentPropDataAsset);
		}
	}
}

// Called every frame
void AIslandsWorldManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AIslandsWorldManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AIslandsWorldManager, WorldSeed);
}

AIslandsWorldManager* AIslandsWorldManager::GetInstance()
{
	check(Instance != nullptr);
	return Instance;
}

FRandomStream& AIslandsWorldManager::GetWorldRandomStream()
{
	return WorldRandomStream;
}

