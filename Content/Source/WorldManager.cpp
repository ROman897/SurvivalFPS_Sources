// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldManager.h"

#include "EnemyCamp.h"
#include "VoxelWorldGenerator.h"
#include "EnvironmentPropDataAsset.h"
#include "EnvironmentPropsInstancer.h"
#include "InstancedEnvironmentPropComponent.h"
#include "MeshColliderOverlap.h"
#include "MyGameInstance.h"
#include "Actors/MercunaNavGrid.h"
#include "Actors/MercunaNavOctree.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMeshActor.h"
#include "FastNoise/VoxelFastNoise.inl"
#include "HAL/Thread.h"
#include "Items/Assets/ItemDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Voronoi/Voronoi.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelTools/VoxelSurfaceTools.h"
#include "VoxelTools/Gen/VoxelSurfaceEditTools.h"

#define GROUND_TRACE_CHANNEL ECC_GameTraceChannel6

AWorldManager* AWorldManager::Instance = nullptr;
	
/* Declaring a stat group that will be visible to the profiler, named Voronoi */
DECLARE_STATS_GROUP(TEXT("WorldManager"), STATGROUP_WorldManager, STATCAT_Advanced);

/* Declaring a cycle stat that belongs to "Orfeas", named Orfeas-FindPrimeNumbers*/
DECLARE_CYCLE_STAT(TEXT("World Manager - Calculate Biomes"), STAT_CalculateBiomesAtVoxels, STATGROUP_WorldManager);
DECLARE_CYCLE_STAT(TEXT("World Manager - Voronoi Compute"), STAT_VoronoiCompute, STATGROUP_WorldManager);

DECLARE_CYCLE_STAT(TEXT("World Manager - Calculate Biomes"), STAT_CalculateBiomesLocalToGlobal, STATGROUP_WorldManager);
DECLARE_CYCLE_STAT(TEXT("World Manager - Find Cell"), STAT_CalculateBiomesFindCell, STATGROUP_WorldManager);
DECLARE_CYCLE_STAT(TEXT("World Manager - Biomes Emplace"), STAT_CalculateBiomesEmplace, STATGROUP_WorldManager);
DECLARE_CYCLE_STAT(TEXT("World Manager - Biomes Index"), STAT_CalculateBiomesIndex, STATGROUP_WorldManager);
// Sets default values
AWorldManager::AWorldManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

const FVoxelPresentBiomesData& AWorldManager::GetBiomesAtVoxelPoint(FVector2D VoxelPoint)
{
	int32 YCoord = FMath::RoundToInt(VoxelPoint.Y);
	int32 XCoord = FMath::RoundToInt(VoxelPoint.X);

	return VoxelsBiomesData[GetVoxelIndex2D(FIntVector(XCoord, YCoord, 0))];
}

void AWorldManager::CalculateBiomesAtVoxels()
{
	SCOPE_CYCLE_COUNTER(STAT_CalculateBiomesAtVoxels);

	int32 NumberOfCores = FGenericPlatformMisc::NumberOfCoresIncludingHyperthreads();
	FThreadSafeCounter ThreadSafeApproxSquareIndexCounter;

	TArray<FThread> Threads;

	for (int32 i = 0; i < NumberOfCores; ++i)
	{
		TUniqueFunction<void()> Func = TUniqueFunction<void()>([&ThreadSafeApproxSquareIndexCounter, this]()
		{
			while (true)
			{
				int32 ApproxSquareId = ThreadSafeApproxSquareIndexCounter.Add(1);
				if (ApproxSquareId >= BiomeApproximationSquares.Num())
				{
					return;
				}
				CalculateBiomesAtApproxSquare(ApproxSquareId);
			}
		});
		
		Threads.Emplace(TEXT("CalculateBiomeWorker"), MoveTemp(Func));
	}

	for (int32 i = 0; i < NumberOfCores; ++i)
	{
		Threads[i].Join();
	}
}

void AWorldManager::CalculateBiomesAtApproxSquare(int32 ApproxSquareId)
{
	FIntVector StartVertexCoords = GetApproxSquareStartCoords(ApproxSquareId);
	const FBiomeApproximationSquare& BiomeApproximationSquare = BiomeApproximationSquares[ApproxSquareId];
	
	for (int32 YCoord = StartVertexCoords.Y; YCoord < StartVertexCoords.Y + BiomeApproximationSquareSize; ++YCoord)
	{
		for (int32 XCoord = StartVertexCoords.X; XCoord < StartVertexCoords.X + BiomeApproximationSquareSize; ++XCoord)
		{
			FVector VoxelLocationInWorldSpace;
			{
				SCOPE_CYCLE_COUNTER(STAT_CalculateBiomesLocalToGlobal);
				FVoxelVector VoxelVector = FVoxelVector(XCoord, YCoord, 0.0f);
				VoxelLocationInWorldSpace = VoxelWorld->LocalToGlobalFloat(VoxelVector);
			}
				
			TArray<FVoxelPossibleBiomeData> AllVoxelBiomesData;

			{
				for (int i = 0; i < BiomeApproximationSquare.PossibleBiomes.Num(); ++i)
				{
					const FBiomeInstance& BiomeInstance = *BiomeApproximationSquare.PossibleBiomes[i];
					
					float XDif = FMath::Abs(BiomeInstance.GlobalLocation.X - VoxelLocationInWorldSpace.X);
					float YDif = FMath::Abs(BiomeInstance.GlobalLocation.Y - VoxelLocationInWorldSpace.Y);
					float Dist = FMath::Pow(FMath::Pow(XDif, BiomeDistanceExponent) + FMath::Pow(YDif, BiomeDistanceExponent), 1.0f / BiomeDistanceExponent);
					float RandomBoundaryNoise = BiomeInstance.Noise.GetPerlin_2D(XCoord, YCoord, BiomeDistanceNoiseFrequency) * BiomeDistanceNoise;
					Dist += RandomBoundaryNoise;
					
					AllVoxelBiomesData.Emplace(FVoxelPossibleBiomeData {Dist, 0.0f, 0.0f, &BiomeInstance} );
				}
			}

			check(AllVoxelBiomesData.Num() > 0);
			Algo::Sort(AllVoxelBiomesData, [](const FVoxelPossibleBiomeData& lhs, const FVoxelPossibleBiomeData& rhs)
			{
				return lhs.Distance < rhs.Distance;
			});

			TArray<FVoxelPossibleBiomeData> RelevantVoxelBiomesData;
			
			RelevantVoxelBiomesData.Add(AllVoxelBiomesData[0]);
			RelevantVoxelBiomesData[0].Strength = 1.0f;
		
			float MinDistance = RelevantVoxelBiomesData[0].Distance;

			float MaxViableDist = MinDistance + MaxBiomeOverlapInWorldUnits;
			
			float StrengthsSum = RelevantVoxelBiomesData[0].Strength;
			
			for (int i = 1; i < AllVoxelBiomesData.Num(); ++i)
			{
				if (MaxViableDist < AllVoxelBiomesData[i].Distance)
				{
					break;
				}

				RelevantVoxelBiomesData.Add(AllVoxelBiomesData[i]);
				float Strength = (MaxViableDist - AllVoxelBiomesData[i].Distance) / MaxBiomeOverlapInWorldUnits;
				RelevantVoxelBiomesData[i].Strength = Strength;
				StrengthsSum += Strength;
			}

			TMap<UBiomeDataAsset*, float> CummulativeBiomes;
			for (const FVoxelPossibleBiomeData& VoxelPossibleBiomeData : RelevantVoxelBiomesData)
			{
				CummulativeBiomes.FindOrAdd(VoxelPossibleBiomeData.BiomeInstance->BiomeRef) += VoxelPossibleBiomeData.Strength;
			}

			TArray<TPair<UBiomeDataAsset*, float>> SortedCummulativeBiomes;
			for (const auto& CummulativeBiome : CummulativeBiomes)
			{
				SortedCummulativeBiomes.Add(CummulativeBiome);
			}

			Algo::Sort(SortedCummulativeBiomes, [](const TPair<UBiomeDataAsset*, float>& lhs, const TPair<UBiomeDataAsset*, float>& rhs)
			{
				return lhs.Value > rhs.Value;
			});

			FVoxelPresentBiomesData VoxelPresentBiomesData;
			
			for (int i = 0; i < FMath::Min(SortedCummulativeBiomes.Num(), MAX_NUMBER_OF_MIXING_BIOMES); ++i)
			{
				float Percentage = SortedCummulativeBiomes[i].Value / StrengthsSum;
				UBiomeDataAsset* BiomeRef = SortedCummulativeBiomes[i].Key;
				//VoxelPresentBiomesData.Biomes.Add(FVoxelPresentBiomeData {Percentage, BiomeRef, BiomeRef->MaterialIndex});
			}

			int32 Index = GetVoxelIndex2D(FIntVector(XCoord, YCoord, 0));
			VoxelsBiomesData[Index] = MoveTemp(VoxelPresentBiomesData);  
		}
	}
}

// this is unused
void AWorldManager::CalculateBiomesAtYCoord(int32 YCoord)
{
	/*
	for (int32 XCoord = VoxelWorld->GetWorldBounds().Min.X; XCoord < VoxelWorld->GetWorldBounds().Max.X; ++XCoord)
	{
		FVector VoxelLocationInWorldSpace;
		{
			SCOPE_CYCLE_COUNTER(STAT_CalculateBiomesLocalToGlobal);
			FVoxelVector VoxelVector = FVoxelVector(XCoord, YCoord, 0.0f);
			VoxelLocationInWorldSpace = VoxelWorld->LocalToGlobalFloat(VoxelVector);
		}
			
		TArray<FVoxelBiomeData> AllVoxelBiomesData;

		{
			for (int i = 0; i < BiomeInstances.Num(); ++i)
			{
				const FBiomeInstance& BiomeInstance = BiomeInstances[i];
				
				float XDif = FMath::Abs(BiomeInstance.GlobalLocation.X - VoxelLocationInWorldSpace.X);
				float YDif = FMath::Abs(BiomeInstance.GlobalLocation.Y - VoxelLocationInWorldSpace.Y);
				float Dist = FMath::Pow(FMath::Pow(XDif, BiomeDistanceExponent) + FMath::Pow(YDif, BiomeDistanceExponent), 1.0f / BiomeDistanceExponent);
				//Dist += BiomeInstance.Noise.GetPerlin_2D(XCoord, YCoord, BiomeDistanceNoiseFrequency) * BiomeDistanceNoise;
				
				AllVoxelBiomesData.Emplace(FVoxelBiomeData {Dist, 0.0f, 0.0f, &BiomeInstance} );
			}
		}
		FVoxelBiomesData RelevantVoxelBiomesData;
		
		check(AllVoxelBiomesData.Num() > 0);

		if (AllVoxelBiomesData.Num() == 1)
		{
			RelevantVoxelBiomesData.Biomes.Add(AllVoxelBiomesData[0]);
			RelevantVoxelBiomesData.Biomes[0].Strength = 1.0f;
			RelevantVoxelBiomesData.Biomes[0].BiomePercentageByStrength = 1.0f;
		} else
		{
			Algo::Sort(AllVoxelBiomesData, [](const FVoxelBiomeData& lhs, const FVoxelBiomeData& rhs)
			{
				return lhs.Distance < rhs.Distance;
			});
			RelevantVoxelBiomesData.Biomes.Add(AllVoxelBiomesData[0]);
			RelevantVoxelBiomesData.Biomes[0].Strength = 1.0f;
		
			float MinDistance = RelevantVoxelBiomesData.Biomes[0].Distance;

			float MaxViableDist = MinDistance + MaxBiomeOverlapInWorldUnits;
			
			float StrengthsSum = RelevantVoxelBiomesData.Biomes[0].Strength;
			
			for (int i = 1; i < FMath::Min(AllVoxelBiomesData.Num(), MAX_NUMBER_OF_MIXING_BIOMES); ++i)
			{
				if (MaxViableDist < AllVoxelBiomesData[i].Distance)
				{
					break;
				}

				RelevantVoxelBiomesData.Biomes.Add(AllVoxelBiomesData[i]);
				float Strength = (MaxViableDist - AllVoxelBiomesData[i].Distance) / MaxBiomeOverlapInWorldUnits;
				RelevantVoxelBiomesData.Biomes[i].Strength = Strength;
				StrengthsSum += Strength;
			}

			for (FVoxelBiomeData& VoxelBiomeData : RelevantVoxelBiomesData.Biomes)
			{
				VoxelBiomeData.BiomePercentageByStrength = VoxelBiomeData.Strength / StrengthsSum;
			}
		}

		int32 Index = GetVoxelIndex2D(FIntVector(XCoord, YCoord, 0));
		VoxelsBiomesData[Index] = MoveTemp(RelevantVoxelBiomesData);
	}
	*/
}

void AWorldManager::SpawnActors()
{
	if (UKismetSystemLibrary::IsServer(GetWorld()))
	{
		UE_LOG(LogTemp, Warning, TEXT("WorldManager spawn actors"));
		GenerateNavigation();
		GenerateEnemyCamps();
		RecalculateWorldCompositionTiles();
		GetWorldTimerManager().SetTimer(RecalculateWorldCompositionTilesHandle, this, &AWorldManager::RecalculateWorldCompositionTiles, RecalculateWorldCompositionTilesTime, true);
		
		UMyGameInstance* MyGameInstance = Cast<UMyGameInstance>(GetGameInstance());
		check(MyGameInstance != nullptr);
		MyGameInstance->Host("MyTestServer");
	}

	bReady = true;
	WorldReadyDelegate.Broadcast();
}

bool AWorldManager::GetSurfaceLocation(FVector& OutLocation)
{
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(GroundTrace), true, GetOwner());
	FVector StartTrace = FVector(OutLocation.X, OutLocation.Y, 100000);
	FVector EndTrace = FVector(OutLocation.X, OutLocation.Y, -100000);

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, GROUND_TRACE_CHANNEL, TraceParams);

	if (Hit.Actor == nullptr)
	{
		return false;
	}
	OutLocation = Hit.Location;
	return true;
}

void AWorldManager::RecalculateNavigationUnderActor(AActor* TargetActor)
{
	TArray<AActor*> NavGrids;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMercunaNavGrid::StaticClass(), NavGrids);
	for (AActor* NavGrid : NavGrids)
	{
		UE_LOG(LogTemp, Warning, TEXT("building nav grid"));
		Cast<AMercunaNavGrid>(NavGrid)->RebuildVolume(TargetActor);
	}
	
	TArray<AActor*> NavOctrees;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMercunaNavOctree::StaticClass(), NavOctrees);
	for (AActor* NavOctree : NavOctrees)
	{
		UE_LOG(LogTemp, Warning, TEXT("building octree"));
		check(Cast<AMercunaNavOctree>(NavOctree) != nullptr);
		Cast<AMercunaNavOctree>(NavOctree)->RebuildVolume(TargetActor);
	}

	return;
	/*RebuildNavigation();
	return;
	TArray<AActor*> NavGrids;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMercunaNavGrid::StaticClass(), NavGrids);
	for (AActor* NavGrid : NavGrids)
	{
		UE_LOG(LogTemp, Warning, TEXT("recalculating navigation"));
		Cast<AMercunaNavGrid>(NavGrid)->RebuildVolume(TargetActor);
	}*/
}

void AWorldManager::RecalculateNavigationInVolumes(const TArray<FBox>& Volumes)
{
	RebuildNavigation();
	return;
	
	TArray<AActor*> NavGrids;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMercunaNavGrid::StaticClass(), NavGrids);
	for (AActor* NavGrid : NavGrids)
	{
		UE_LOG(LogTemp, Warning, TEXT("recalculating navigation"));
		Cast<AMercunaNavGrid>(NavGrid)->RebuildVolumes(Volumes);
	}
}

void AWorldManager::RecalculateNavigationInVoxelBox(const FVoxelIntBox VoxelIntBox)
{
	RebuildNavigation();
	return;
	RecalculateNavigationInVoxelBoxes({VoxelIntBox});
}

void AWorldManager::RecalculateNavigationInVoxelBoxes(const TArray<FVoxelIntBox>& VoxelIntBoxes)
{
	RebuildNavigation();
	return;
	UE_LOG(LogTemp, Warning, TEXT("recalculating navigation in boxes"));
	TArray<FBox> ModifiedBoxes;
	ModifiedBoxes.Reserve(VoxelIntBoxes.Num());

	for (const FVoxelIntBox& VoxelIntBox : VoxelIntBoxes)
	{
		FVector GlobalEditBoundsMinLocation = VoxelWorld->LocalToGlobal(VoxelIntBox.Min);
		FVector GlobalEditBoundsMaxLocation = VoxelWorld->LocalToGlobal(VoxelIntBox.Max);

		ModifiedBoxes.Emplace(GlobalEditBoundsMinLocation, GlobalEditBoundsMaxLocation);
	}
	RecalculateNavigationInVolumes(ModifiedBoxes);
}

void AWorldManager::DebugCalculateBiomesAtLocation(FVector Location)
{
	/*FIntVector VoxelVector = VoxelWorld->GlobalToLocal(Location);
	FVector VoxelLocationInWorldSpace = VoxelWorld->LocalToGlobalFloat(VoxelVector);

	UE_LOG(LogTemp, Warning, TEXT("voxel coords: %s  | World pos: %s"), *VoxelVector.ToString(), *VoxelLocationInWorldSpace.ToString());
		
	TArray<FVoxelBiomeData> AllVoxelBiomesData;

	for (int i = 0; i < BiomeInstances.Num(); ++i)
	{
		const FBiomeInstance& BiomeInstance = BiomeInstances[i];
		
		float XDif = FMath::Abs(BiomeInstance.GlobalLocation.X - VoxelLocationInWorldSpace.X);
		float YDif = FMath::Abs(BiomeInstance.GlobalLocation.Y - VoxelLocationInWorldSpace.Y);
		float Dist = FMath::Pow(FMath::Pow(XDif, BiomeDistanceExponent) + FMath::Pow(YDif, BiomeDistanceExponent), 1.0f / BiomeDistanceExponent);
		//Dist += BiomeInstance.Noise.GetPerlin_2D(XCoord, YCoord, BiomeDistanceNoiseFrequency) * BiomeDistanceNoise;
		
		AllVoxelBiomesData.Emplace(FVoxelBiomeData {Dist, 0.0f, 0.0f, &BiomeInstance} );
	}
	
	FVoxelBiomesData RelevantVoxelBiomesData;
	
	check(AllVoxelBiomesData.Num() > 0);

	Algo::Sort(AllVoxelBiomesData, [](const FVoxelBiomeData& lhs, const FVoxelBiomeData& rhs)
	{
		return lhs.Distance < rhs.Distance;
	});

	for (int i = 0; i < 3; ++i)
	{
		UE_LOG(LogTemp, Warning, TEXT("%i th biome: %i  with distance: %f"), i, AllVoxelBiomesData[i].BiomeInstance->BiomeInstanceId, AllVoxelBiomesData[i].Distance);
	}
	*/
}

void AWorldManager::RegisterPlayer(AActor* Player)
{
	UE_LOG(LogTemp, Warning, TEXT("world manager registered new player %s"), *Player->GetName());
	RegisteredPlayers.Add(Player);
	RecalculateWorldCompositionTiles();
}

FHitResult AWorldManager::LineTraceGround(FVector2D Location)
{
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(GroundTrace), true, GetOwner());
	FVector StartTrace = FVector(Location.X, Location.Y, 100000);
	FVector EndTrace = FVector(Location.X, Location.Y, -100000);

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, GROUND_TRACE_CHANNEL, TraceParams);
	return Hit;
}

FRandomStream& AWorldManager::GetRandomStream()
{
	return WorldRandomStream;
}

const TArray<UItemDataAsset*>& AWorldManager::GetAllItemDataAssets()
{
	if (AllItemDataAssets.Num() > 0)
	{
		return AllItemDataAssets;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetsData;
	FARFilter Filter;
	Filter.ClassNames.Add(FName(UItemDataAsset::StaticClass()->GetName()));
	Filter.PackagePaths.Add("/Game/DataAssets/Items");
	Filter.bRecursiveClasses = true;
	AssetRegistryModule.Get().GetAssets(Filter, AssetsData);

	for (const FAssetData AssetData : AssetsData)
	{
		UItemDataAsset* AtomicItemDataAsset = Cast<UItemDataAsset>(AssetData.GetAsset());
		check(AtomicItemDataAsset != nullptr);
		AllItemDataAssets.Add(AtomicItemDataAsset);
	}

	return AllItemDataAssets;
}

// Called when the game starts or when spawned
void AWorldManager::BeginPlay()
{
	if (UKismetSystemLibrary::IsServer(GetWorld()))
	{
		GenerateNewWorld();
	} else
	{
		GenerateWorldFromSeed();
	}
	
	Super::BeginPlay();
}

void AWorldManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWorldManager, WorldSeed);
	DOREPLIFETIME(AWorldManager, SyncedTerrainModifications);
}

void AWorldManager::OnRepSyncedTerrainModifications()
{
	for (uint32 Counter = SyncedTerrainModifications.LastSyncedCounter; Counter < SyncedTerrainModifications.Counter; ++Counter)
	{
		const FTerrainModification& TerrainModification = SyncedTerrainModifications.TerrainModifications[Counter];
		switch (TerrainModification.TerrainModificationType)
		{
			case ETerrainModificationType::FLATTEN:
				Flatten(TerrainModification);
				break;
			case ETerrainModificationType::DIG:
				Dig(TerrainModification);
				break;
			default:
				check(false);
		}
	}
	SyncedTerrainModifications.LastSyncedCounter = SyncedTerrainModifications.Counter;
}

void AWorldManager::Flatten(const FTerrainModification& TerrainModification)
{
	auto EditsStack = UVoxelSurfaceTools::AddToStack(FVoxelSurfaceEditsStack(), UVoxelSurfaceTools::ApplyConstantStrength(TerrainModification.Strength));

	EditsStack = UVoxelSurfaceTools::AddToStack(EditsStack, UVoxelSurfaceTools::ApplyFalloff(AWorldManager::GetInstance()->GetVoxelWorld(), TerrainModification.VoxelFalloff, TerrainModification.Location, TerrainModification.Radius, TerrainModification.Falloff));
	EditsStack = UVoxelSurfaceTools::AddToStack(EditsStack, UVoxelSurfaceTools::ApplyFlatten(AWorldManager::GetInstance()->GetVoxelWorld(), TerrainModification.Location2));

	auto EditBounds = UVoxelBlueprintLibrary::MakeIntBoxFromGlobalPositionAndRadius(AWorldManager::GetInstance()->GetVoxelWorld(), TerrainModification.Location, TerrainModification.Radius + TerrainModification.Falloff);

	FVoxelSurfaceEditsVoxels EditsVoxels;
	UVoxelSurfaceTools::FindSurfaceVoxels(EditsVoxels, GetVoxelWorld(), EditBounds);
	auto ProcessedVoxels = UVoxelSurfaceTools::ApplyStack(EditsVoxels, EditsStack);

	UVoxelSurfaceEditTools::EditVoxelValues(GetVoxelWorld(), ProcessedVoxels);

	//ModifiedVoxelBoxes.Add(EditBounds);
}

void AWorldManager::Dig(const FTerrainModification& TerrainModification)
{
	UE_LOG(LogTemp, Warning, TEXT("server digoo"));
	auto EditsStack = UVoxelSurfaceTools::AddToStack(FVoxelSurfaceEditsStack(), UVoxelSurfaceTools::ApplyConstantStrength(TerrainModification.Strength));
	EditsStack = UVoxelSurfaceTools::AddToStack(EditsStack, UVoxelSurfaceTools::ApplyFalloff(GetVoxelWorld(), TerrainModification.VoxelFalloff, TerrainModification.Location, TerrainModification.Radius, TerrainModification.Falloff));
	auto EditBounds = UVoxelBlueprintLibrary::MakeIntBoxFromGlobalPositionAndRadius(GetVoxelWorld(), TerrainModification.Location, TerrainModification.Radius + TerrainModification.Falloff);

	FVoxelSurfaceEditsVoxels EditsVoxels;
	UVoxelSurfaceTools::FindSurfaceVoxels(EditsVoxels, GetVoxelWorld(), EditBounds);
	auto ProcessedVoxels = UVoxelSurfaceTools::ApplyStack(EditsVoxels, EditsStack);

	UVoxelSurfaceEditTools::EditVoxelValues(GetVoxelWorld(), ProcessedVoxels);
}

int32 AWorldManager::GetVoxelIndex2D(FIntVector VoxelCoords)
{
	return (VoxelCoords.Y + VoxelWorld->WorldSizeInVoxel / 2) * VoxelWorld->WorldSizeInVoxel + VoxelCoords.X + VoxelWorld->WorldSizeInVoxel / 2;
}

FIntVector AWorldManager::GetApproxSquareStartCoords(int ApproxSquareId)
{
	FIntVector SquareCoords = FIntVector(ApproxSquareId % BiomeApproximationSquaresPerSide, ApproxSquareId / BiomeApproximationSquaresPerSide, 0);

	FIntVector UncenteredVoxelStartCoords = SquareCoords * BiomeApproximationSquareSize;

	FIntVector CenteredVoxelStartCoords = UncenteredVoxelStartCoords - VoxelWorld->WorldSizeInVoxel / 2;
	return CenteredVoxelStartCoords;
}

void AWorldManager::TerrainGenerated()
{
	GetWorldTimerManager().SetTimer(SpawnEnvironmentDelayHandle, this, &AWorldManager::GenerateNewEnvironment, 5.0f, false);
}

void AWorldManager::GenerateNewEnvironment()
{
	GetWorldTimerManager().SetTimer(SpawnActorsDelayHandle, this, &AWorldManager::SpawnActors, 3.0f);
	return;
	UE_LOG(LogTemp, Warning, TEXT("generate new environment"));
	AddHierarchicalInstancedMeshComponents();
	
	AMeshColliderOverlap* CollisionTestActor = GetWorld()->SpawnActor<AMeshColliderOverlap>(AMeshColliderOverlap::StaticClass());
	check(CollisionTestActor != nullptr);
				
	
	for (int32 Y = VoxelWorld->GetWorldBounds().Min.Y; Y < VoxelWorld->GetWorldBounds().Max.Y; Y += AtomicEnvironmentSubregionSizeInVoxels)
	{
		for (int32 X = VoxelWorld->GetWorldBounds().Min.X; X < VoxelWorld->GetWorldBounds().Max.X; X += AtomicEnvironmentSubregionSizeInVoxels)
		{
			int32 EndX = FMath::Min(X + AtomicEnvironmentSubregionSizeInVoxels - 1, VoxelWorld->GetWorldBounds().Max.X - 1);
			int32 EndY = FMath::Min(Y + AtomicEnvironmentSubregionSizeInVoxels - 1, VoxelWorld->GetWorldBounds().Max.Y - 1);

			FVector2D Center = FVector2D((X + EndX ) / 2, (Y + EndY) / 2);

			const FVoxelPresentBiomesData VoxelBiomesData = GetBiomesAtVoxelPoint(Center);

			// there should always be some present at any world location
			check(VoxelBiomesData.Biomes.Num() > 0);

			const float VoxelSizeInMeters = VoxelWorld->VoxelSize / 100;
			const float AreaInMeters = (EndX - X) * VoxelSizeInMeters * (EndY - Y) * VoxelSizeInMeters;
			const float AreaMultiplier = AreaInMeters / 100; 

			TArray<const UEnvironmentPropDataAsset*> EnvironmentPropDataAssets;
			TArray<float> CumFrequencies;
			
			float TotalFrequency = 0.0f;
			
			for (const FVoxelPresentBiomeData& VoxelBiomeData : VoxelBiomesData.Biomes)
			{
				for (const FEnvironmentPropSpawnAttributes& EnvironmentPropSpawnAttributes : VoxelBiomeData.BiomeRef->SpawnableEnvironmentProps)
				{
					float ScaledFrequency = EnvironmentPropSpawnAttributes.EnvironmentPropFrequencyPer100m2 * AreaMultiplier * VoxelBiomeData.BiomePercentageByStrength;
					TotalFrequency += ScaledFrequency;
					CumFrequencies.Add(TotalFrequency);
					EnvironmentPropDataAssets.Add(EnvironmentPropSpawnAttributes.EnvironmentPropDataAsset);
				}
			}

			int32 NumberOfPropsToSpawn = FMath::RoundToInt(TotalFrequency);
			
			for (int32 i = 0; i < NumberOfPropsToSpawn; ++i)
			{
				float RandFloat = WorldRandomStream.FRandRange(0.0f, TotalFrequency);
				int ChosenIndex = Algo::LowerBound(CumFrequencies, RandFloat);

				const UEnvironmentPropDataAsset* ChosenProp = EnvironmentPropDataAssets[ChosenIndex];
				
				FVector StartVec3D = VoxelWorld->LocalToGlobal(FIntVector(X, Y, 0));
				FVector EndVec3D = VoxelWorld->LocalToGlobal(FIntVector(EndX, EndY, 0));
				
				float TargetX = WorldRandomStream.FRandRange(StartVec3D.X, EndVec3D.X);
				float TargetY = WorldRandomStream.FRandRange(StartVec3D.Y, EndVec3D.Y);

				FVector2D Location2D(TargetX, TargetY);

				if (FVector2D::DistSquared(Location2D, FVector2D::ZeroVector) < ClearDistanceFromPlayerSpawnSqr)
				{
					continue;
				}

				FHitResult GroundHit = LineTraceGround(Location2D);
				
				if (GroundHit.Actor != GetVoxelWorld())
				{
					UE_LOG(LogTemp, Warning, TEXT("can't add instance didn't hit anything at %s"), *Location2D.ToString());
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

				if (BotLeftHit.Actor != GetVoxelWorld()
					|| TopLeftHit.Actor != GetVoxelWorld()
					|| BotRightHit.Actor != GetVoxelWorld()
					|| TopRightHit.Actor != GetVoxelWorld())
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

				//FVector ColliderLocation = FinalLocation + FVector::UpVector * MeshBoundingBox.GetExtent().Z / 2;
				FActorSpawnParameters ColliderSpawnParameters;
				ColliderSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				TArray<FOverlapResult> Overlaps;
				
				CollisionTestActor->GetStaticMeshComponent()->SetStaticMesh(ChosenProp->PropStaticMesh);
				GetWorld()->ComponentOverlapMulti(Overlaps, CollisionTestActor->GetStaticMeshComponent(), FinalLocation, FinalRotation);
				
				bool InvalidOverlap = false;
				
				for (const FOverlapResult& Overlap : Overlaps)
				{
					if (Overlap.Actor != GetVoxelWorld())
					{
						//FVector OverlapLocation = Overlap.;
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
					EnvironmentPropDataAssets[ChosenIndex], Transform);
			}
		}
	}
	
	CollisionTestActor->Destroy();
	
	GetWorldTimerManager().SetTimer(SpawnActorsDelayHandle, this, &AWorldManager::SpawnActors, 3.0f);
}

void AWorldManager::CalculateBiomeApproximationSquares()
{
	TArray<FVector> VoronoiSites;
	for (const FBiomeInstance& BiomeInstance : BiomeInstances)
	{
		VoronoiSites.Add(FVector(BiomeInstance.GlobalLocation, 0.0f));
	}
	FVector Min = FVector(VoxelWorld->GetWorldBounds().Min.X, VoxelWorld->GetWorldBounds().Min.Y, 0.0f) * VoxelWorld->VoxelSize;
	FVector Max = FVector(VoxelWorld->GetWorldBounds().Max.X, VoxelWorld->GetWorldBounds().Max.Y, 0.0f) * VoxelWorld->VoxelSize;
	FBox Box = FBox(Min, Max);
	FVoronoiDiagram VoronoiDiagram (VoronoiSites, Box, 0.0f);
	TArray<FVoronoiCellInfo> VoronoiCells;
	{
		VoronoiDiagram.ComputeAllCells(VoronoiCells);
	}
	
	const float BiomeApproximationMaxDist = BiomeApproximationSquareSize * FMath::Sqrt(2.0f);

	for (int32 ApproxSquareId = 0; ApproxSquareId < BiomeApproximationSquares.Num(); ++ApproxSquareId)
	{
		FIntVector SquareStartCoords = GetApproxSquareStartCoords(ApproxSquareId);
		FIntVector SquareEndCoords = SquareStartCoords + FIntVector(BiomeApproximationSquareSize, BiomeApproximationSquareSize, 0);
		
		FVector StartSquareLocationInWorldSpace = VoxelWorld->LocalToGlobalFloat(SquareStartCoords);
		FVector EndSquareLocationInWorldSpace = VoxelWorld->LocalToGlobalFloat(SquareEndCoords);

		FVector CenterLocationInWorldSpace = (StartSquareLocationInWorldSpace + EndSquareLocationInWorldSpace) / 2;
		CenterLocationInWorldSpace.Z = 0;
		FVector2D CenterLocationInWorldSpace2D(CenterLocationInWorldSpace); 

		int ClosestBiomeVoronoiCellId;

		// we find the voronoi cell corresponding to biome that the center point falls into
		{
			ClosestBiomeVoronoiCellId = VoronoiDiagram.FindCell(CenterLocationInWorldSpace);
			check(ClosestBiomeVoronoiCellId != -1);
		}

		FBiomeApproximationSquare BiomeApproximationSquare;
		BiomeApproximationSquare.PossibleBiomes.Add(&BiomeInstances[ClosestBiomeVoronoiCellId]);
		const FVoronoiCellInfo& VoronoiCellInfo = VoronoiCells[ClosestBiomeVoronoiCellId];

		float DistFromClosestBiome = FVector2D::Distance(BiomeInstances[ClosestBiomeVoronoiCellId].GlobalLocation, CenterLocationInWorldSpace2D);
		float MaxPossibleDistanceFromBiome = DistFromClosestBiome + MaxBiomeOverlapInWorldUnits + BiomeApproximationMaxDist + BiomeDistanceNoise;

		// the only possible other biomes we consider are neighboring voronoi cells
		for (int NeighborVoronoiCellId : VoronoiCellInfo.Neighbors)
		{
			if (NeighborVoronoiCellId < 0)
			{
				continue;
			}
			const FBiomeInstance& NeighborBiomeInstance = BiomeInstances[NeighborVoronoiCellId];
			float DistFromThisBiome = FVector2D::Distance(NeighborBiomeInstance.GlobalLocation, CenterLocationInWorldSpace2D);
			if (DistFromThisBiome > MaxPossibleDistanceFromBiome)
			{
				//continue;
			}
			BiomeApproximationSquare.PossibleBiomes.Add(&NeighborBiomeInstance);
		}

		BiomeApproximationSquares[ApproxSquareId] = MoveTemp(BiomeApproximationSquare);
	}
}

void AWorldManager::GenerateNavigation()
{
	TArray<AActor*> NavGrids;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMercunaNavGrid::StaticClass(), NavGrids);
	for (AActor* NavGrid : NavGrids)
	{
		UE_LOG(LogTemp, Warning, TEXT("building nav grid"));
		Cast<AMercunaNavGrid>(NavGrid)->Build();
	}
	
	TArray<AActor*> NavOctrees;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMercunaNavOctree::StaticClass(), NavOctrees);
	for (AActor* NavOctree : NavOctrees)
	{
		UE_LOG(LogTemp, Warning, TEXT("building octree"));
		check(Cast<AMercunaNavOctree>(NavOctree) != nullptr);
		Cast<AMercunaNavOctree>(NavOctree)->Build();

		//Cast<AMercunaNavOctree>(NavOctree)->OnBuildComplete.AddDynamic(this, &AWorldManager::NavOctreeBuildComplete);
	}
}

void AWorldManager::RebuildNavigation()
{
	TArray<AActor*> NavGrids;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMercunaNavGrid::StaticClass(), NavGrids);
	// TODO(Roman): rebuild changes is used if you modify volumes themselves, not what's in them
	for (AActor* NavGrid : NavGrids)
	{
		UE_LOG(LogTemp, Warning, TEXT("building nav grid"));
		Cast<AMercunaNavOctree>(NavGrid)->RebuildChanges();
	}
	
	TArray<AActor*> NavOctrees;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMercunaNavOctree::StaticClass(), NavOctrees);
	for (AActor* NavOctree : NavOctrees)
	{
		UE_LOG(LogTemp, Warning, TEXT("building octree"));
		check(Cast<AMercunaNavOctree>(NavOctree) != nullptr);
		Cast<AMercunaNavOctree>(NavOctree)->RebuildChanges();
	}
}

void AWorldManager::GenerateEnemyCamps()
{
	for (int32 Y = VoxelWorld->GetWorldBounds().Min.Y; Y < VoxelWorld->GetWorldBounds().Max.Y; Y += AtomicEnvironmentSubregionSizeInVoxels)
	{
		for (int32 X = VoxelWorld->GetWorldBounds().Min.X; X < VoxelWorld->GetWorldBounds().Max.X; X += AtomicEnvironmentSubregionSizeInVoxels)
		{
			int32 EndX = FMath::Min(X + AtomicEnvironmentSubregionSizeInVoxels - 1, VoxelWorld->GetWorldBounds().Max.X - 1);
			int32 EndY = FMath::Min(Y + AtomicEnvironmentSubregionSizeInVoxels - 1, VoxelWorld->GetWorldBounds().Max.Y - 1);

			FVector2D Center = FVector2D((X + EndX ) / 2, (Y + EndY) / 2);

			const FVoxelPresentBiomesData& VoxelBiomesData = GetBiomesAtVoxelPoint(Center);

			// there should always be some present at any world location
			check(VoxelBiomesData.Biomes.Num() > 0);

			float DesiredDensity = (EndX - X + 1) * (EndY - Y + 1) * VoxelWorld->VoxelSize
			* VoxelBiomesData.Biomes[0].BiomeRef->EnemyCampsFrequencyPer100m2
			/ 100000000;
			float LowerDensity = FMath::FloorToFloat(DesiredDensity);
			float UpperDensity = FMath::CeilToFloat(DesiredDensity);
			float RandDensity = WorldRandomStream.FRandRange(LowerDensity, UpperDensity);

			float TargetCount;
			if (DesiredDensity > RandDensity)
			{
				TargetCount = UpperDensity;
			} else
			{
				TargetCount= LowerDensity;
			}

			for (int i = 0; i < TargetCount; ++i)
			{
				FVector StartVec3D = VoxelWorld->LocalToGlobal(FIntVector(X, Y, 0));
				FVector EndVec3D = VoxelWorld->LocalToGlobal(FIntVector(EndX, EndY, 0));
				
				float TargetX = WorldRandomStream.FRandRange(StartVec3D.X, EndVec3D.X);
				float TargetY = WorldRandomStream.FRandRange(StartVec3D.Y, EndVec3D.Y);

				FVector2D Target = FVector2D(TargetX, TargetY);

				if (FVector2D::DistSquared(Target, FVector2D::ZeroVector) < EnemyCampMinDistanceFromPlayerSpawnSqr)
				{
					continue;
				}
				
				FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(GroundTrace), true, GetOwner());
				FVector StartTrace = FVector(TargetX, TargetY, 100000);
				FVector EndTrace = FVector(TargetX, TargetY, -100000);

				FHitResult Hit(ForceInit);
				GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, GROUND_TRACE_CHANNEL, TraceParams);

				if (Hit.Actor == nullptr)
				{
					continue;
				}
				TArray<float> EnemyCampProbabilities;
				const auto& EnemyCamps = VoxelBiomesData.Biomes[0].BiomeRef->EnemyCamps;

				float CumProbability = 0.0f;
				for (const FEnemyCampData& EnemyCamp : EnemyCamps)
				{
					CumProbability += EnemyCamp.Probability;
					EnemyCampProbabilities.Add(CumProbability);
				}

				float RandNum = WorldRandomStream.FRandRange(0.0f, 1.0f);
				int32 EnemyCampIndex = Algo::LowerBound(EnemyCampProbabilities, RandNum);
				if (EnemyCampIndex >= EnemyCampProbabilities.Num())
				{
					continue;
				}

				FIntPoint WorldCompositionTileCoords = WorldToTile(Hit.Location);

				UE_LOG(LogTemp, Warning, TEXT("spawning enemy camp at location: %s  at tile: %s"), *Hit.Location.ToString(), *WorldCompositionTileCoords.ToString());
				FWorldCompositionTile& WorldCompositionTile = WorldCompositionTiles.FindOrAdd(WorldCompositionTileCoords);
				WorldCompositionTile.SerializedEnemyCamps.Emplace(FSerializedEnemyCamp {Hit.Location, EnemyCamps[EnemyCampIndex]} );
				
				/*AEnemyCamp* SpawnedCamp = GetWorld()->SpawnActor<AEnemyCamp>(EnemyCampClass, Hit.Location, FQuat::Identity.Rotator());
				check(SpawnedCamp != nullptr);
				SpawnedCamp->SetEnemyCamp(EnemyCamps[EnemyCampIndex]);*/
			}
		}
	}
}

void AWorldManager::GenerateBiomeInstancesOrigins()
{
	check(TemperatureByYCoordCurve);
	
	for (int i = 0; i < NumberOfBiomeInstances; ++i)
	{
		float X = WorldRandomStream.FRandRange(-0.5f, 0.5f);
		float Y = WorldRandomStream.FRandRange(-0.5f, 0.5f);

		float Temperature = TemperatureByYCoordCurve->GetFloatValue(Y);

		TArray<float> BiomeCumStrengths;

		float StrengthsSum = 0.0f;
		for (const UBiomeDataAsset* Biome : AllBiomes)
		{
			float Strength = Biome->BiomeStrengthByTemperatureCurve->GetFloatValue(Temperature);
			StrengthsSum += Strength;
			BiomeCumStrengths.Add(StrengthsSum);
		}

		float RandStrength = WorldRandomStream.FRandRange(0.0f, StrengthsSum);
		int ChosenBiome = Algo::LowerBound(BiomeCumStrengths, RandStrength);
		BiomeInstances.Add(FBiomeInstance {AllBiomes[ChosenBiome], FVector2D(X, Y) * VoxelWorld->VoxelSize * VoxelWorld->WorldSizeInVoxel} );
		BiomeInstances.Last().Noise.SetSeed(WorldRandomStream.RandRange(0, TNumericLimits<int32>::Max() - 1));
		BiomeInstances.Last().BiomeInstanceId = i;
		UE_LOG(LogTemp, Warning, TEXT("spawning biome %s with temperature %f.  at: %s"), *AllBiomes[ChosenBiome]->GetName(), Temperature, *FVector2D(X, Y).ToString());

		FVector WorldPos = FVector(X, Y, 0) * VoxelWorld->WorldSizeInVoxel * VoxelWorld->VoxelSize;
		AActor* DebugActor = GetWorld()->SpawnActor(BiomeOriginDebugActor, &WorldPos);

		FString BiomeActorName = AllBiomes[ChosenBiome]->GetName() + FString::FromInt(i);
		//DebugActor->SetActorLabel(*BiomeActorName);
		
	}
	check(BiomeInstances.Num() > 0);
}

void AWorldManager::AddHierarchicalInstancedMeshComponents()
{
	for (const UBiomeDataAsset* Biome : AllBiomes)
	{
		for (FEnvironmentPropSpawnAttributes EnvironmentPropSpawnAttributes : Biome->SpawnableEnvironmentProps)
		{
			UE_LOG(LogTemp, Warning, TEXT("register instancee"));
			AEnvironmentPropsInstancer::GetInstance()->RegisterEnvironmentProp(EnvironmentPropSpawnAttributes.EnvironmentPropDataAsset);
		}
	}
}

void AWorldManager::LoadTile(FIntPoint TileCoord)
{
	UE_LOG(LogTemp, Warning, TEXT("loading tile: %s"), *TileCoord.ToString());
	FWorldCompositionTile* WorldCompositionTile = WorldCompositionTiles.Find(TileCoord);
	check(WorldCompositionTile);

	for (const FSerializedEnemyCamp& SerializedEnemyCamp : WorldCompositionTile->SerializedEnemyCamps)
	{
		AEnemyCamp* EnemyCamp = GetWorld()->SpawnActor<AEnemyCamp>(EnemyCampClass, SerializedEnemyCamp.Location, FRotator::ZeroRotator);
		EnemyCamp->SetEnemyCamp(SerializedEnemyCamp.EnemyCampData);
		WorldCompositionTile->SpawnedEnemyCamps.Add(EnemyCamp);
	}
	
	WorldCompositionLoadedTiles.Add(TileCoord);
}

void AWorldManager::UnloadTile(FIntPoint TileCoord)
{
	UE_LOG(LogTemp, Warning, TEXT("Unloading tile: %s"), *TileCoord.ToString());
	FWorldCompositionTile* WorldCompositionTile = WorldCompositionTiles.Find(TileCoord);
	check(WorldCompositionTile);
	
	for (AEnemyCamp* EnemyCamp : WorldCompositionTile->SpawnedEnemyCamps)
	{
		EnemyCamp->Destroy();
	}
	WorldCompositionTile->SpawnedEnemyCamps.Empty();
	WorldCompositionLoadedTiles.Remove(TileCoord);
}

FIntPoint AWorldManager::WorldToTile(FVector Location)
{
	int32 TileX = FMath::Floor(Location.X / WorldCompositionTileSize);
	int32 TileY = FMath::Floor(Location.Y / WorldCompositionTileSize);
	return FIntPoint(TileX, TileY);
}

void AWorldManager::NavOctreeBuildComplete(bool Successful)
{
	if (Successful)
	{
		UE_LOG(LogTemp, Warning, TEXT("nav octree build successful"))
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("nav octree build failed"))
	}
}

// Called every frame
void AWorldManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

AVoxelWorld* AWorldManager::GetVoxelWorld()
{
	return VoxelWorld;
}

void AWorldManager::GenerateNewWorld()
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

void AWorldManager::GenerateWorldFromSeed()
{
	check(VoxelWorldClass != nullptr);
	VoxelWorld = GetWorld()->SpawnActor<AVoxelWorld>(VoxelWorldClass);
	WorldRandomStream.Initialize(WorldSeed);
	UE_LOG(LogTemp, Warning, TEXT("generated world with seed: %i"), WorldRandomStream.GetInitialSeed());
	
	EnemyCampMinDistanceFromPlayerSpawnSqr = EnemyCampMinDistanceFromPlayerSpawn * EnemyCampMinDistanceFromPlayerSpawn;
	VoxelsBiomesData.SetNum(VoxelWorld->WorldSizeInVoxel * VoxelWorld->WorldSizeInVoxel);
	
	BiomeApproximationSquaresPerSide = VoxelWorld->WorldSizeInVoxel / BiomeApproximationSquareSize;
	BiomeApproximationSquares.SetNum(BiomeApproximationSquaresPerSide * BiomeApproximationSquaresPerSide);

	ClearDistanceFromPlayerSpawnSqr = ClearDistanceFromPlayerSpawn * ClearDistanceFromPlayerSpawn;
	
	GenerateBiomeInstancesOrigins();
	CalculateBiomeApproximationSquares();
	CalculateBiomesAtVoxels();
	
	VoxelWorld->OnWorldLoaded.AddDynamic(this, &AWorldManager::TerrainGenerated);
	if (!VoxelWorld->Generator.IsValid())
	{
		VoxelWorld->Generator.Type = EVoxelGeneratorPickerType::Class;
		VoxelWorld->Generator.Class = UVoxelWorldGenerator::StaticClass();
	}

	VoxelWorld->CreateWorld();
}

void AWorldManager::AddTerrainModification(const FTerrainModification& TerrainModification)
{
	SyncedTerrainModifications.TerrainModifications.Add(TerrainModification);
	SyncedTerrainModifications.Counter++;
	OnRepSyncedTerrainModifications();
	UE_LOG(LogTemp, Warning, TEXT("server rpc add terrain modification"));
}

void AWorldManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void AWorldManager::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	Instance = this;
}

bool AWorldManager::IsReady()
{
	return bReady;
}

void AWorldManager::RecalculateWorldCompositionTiles()
{
	TSet<FIntPoint> TilesToLoad;

	for (AActor* Player : RegisteredPlayers)
	{
		UE_LOG(LogTemp, Warning, TEXT("recalculate world composition for player: %s"), *Player->GetName());
		FIntPoint CenterTile = WorldToTile(Player->GetActorLocation());
		
		for (int32 TileY = CenterTile.Y - WorldCompositionNumTilesToLoadAroundPlayer; TileY <= CenterTile.Y + WorldCompositionNumTilesToLoadAroundPlayer; ++TileY)
		{
			for (int32 TileX = CenterTile.X - WorldCompositionNumTilesToLoadAroundPlayer; TileX <= CenterTile.X + WorldCompositionNumTilesToLoadAroundPlayer; ++TileX)
			{
				if (!WorldCompositionTiles.Contains(FIntPoint(TileX, TileY)))
				{
					continue;
				}
				TilesToLoad.Add(FIntPoint(TileX, TileY));
			}
		}
	}

	for (FIntPoint NewTileToLoad : TilesToLoad.Difference(WorldCompositionLoadedTiles))
	{
		LoadTile(NewTileToLoad);
	}

	for (FIntPoint OldTileToUnload : WorldCompositionLoadedTiles.Difference(TilesToLoad))
	{
		UnloadTile(OldTileToUnload);
	}
}
