// Fill out your copyright notice in the Description page of Project Settings.


#include "SplineCreatorComponent.h"
#include "IndexSplineMeshComponent.h"
#include <UObject/UObjectGlobals.h>
#include <Engine/StaticMesh.h>

void USplineCreatorComponent::SpawnSpline(const TArray<class UMaterialInterface*>& Materials)
{
	for (UIndexSplineMeshComponent* SpawnedSplineMesh : SpawnedSplineMeshes) {
		SpawnedSplineMesh->DestroyComponent();
	}
	SpawnedSplineMeshes.Empty();

	float SplineLength = GetSplineLength();
	int32 NumberOfParts = FMath::RoundToInt(SplineLength / MeshStretchSize);
	float UniformMeshStretchSize = SplineLength / NumberOfParts;
	
	for (int i = 0; i < NumberOfParts; ++i) {
		FName Name = FName(FString("SplineMeshComponent") + FString::FromInt(i));
		UIndexSplineMeshComponent* SplineMeshComponent = NewObject<UIndexSplineMeshComponent>(this, UIndexSplineMeshComponent::StaticClass(), Name);
		SplineMeshComponent->RegisterComponent();

		SplineMeshComponent->Id = i;
		SplineMeshComponent->bIsLast = i == NumberOfParts - 1;

		// TODO(Roman): this is necessary because we cannot assign static mesh at runtime,
		// we get error that cannot assign mesh to static object
		// but in fact this is static when spawned
		SplineMeshComponent->SetMobility(EComponentMobility::Movable);
		SplineMeshComponent->SetStaticMesh(ConveyorStaticMesh);
		
		SplineMeshComponent->SetStartScale(SplineScale);
		SplineMeshComponent->SetEndScale(SplineScale);

		SplineMeshComponent->SetCollisionProfileName(CollisionProfileName);

		GetOwner()->AddInstanceComponent(SplineMeshComponent);

		for (int32 MaterialIndex = 0; MaterialIndex < Materials.Num(); ++MaterialIndex)
		{
			SplineMeshComponent->SetMaterial(MaterialIndex, Materials[MaterialIndex]);
		}

		FVector StartLocation = GetLocationAtDistanceAlongSpline(i * UniformMeshStretchSize, ESplineCoordinateSpace::World);
		FVector EndLocation = GetLocationAtDistanceAlongSpline((i + 1) * UniformMeshStretchSize, ESplineCoordinateSpace::World);

		FVector StartTangent = GetTangentAtDistanceAlongSpline(i * UniformMeshStretchSize, ESplineCoordinateSpace::World);
		FVector EndTangent = GetTangentAtDistanceAlongSpline((i + 1) * UniformMeshStretchSize, ESplineCoordinateSpace::World);

		SplineMeshComponent->SetStartAndEnd(StartLocation, StartTangent, EndLocation, EndTangent);

		SpawnedSplineMeshes.Add(SplineMeshComponent);
	}
}

TArray<UIndexSplineMeshComponent*> USplineCreatorComponent::GetSpawnedSplineMeshes()
{
	return SpawnedSplineMeshes;
}

void USplineCreatorComponent::SetMaterials(const TArray<UMaterialInterface*>& Materials)
{
	for (UIndexSplineMeshComponent* SpawnedSplineMesh : SpawnedSplineMeshes)
	{
		for (int32 i = 0; i < Materials.Num(); ++i)
		{
			SpawnedSplineMesh->SetMaterial(i, Materials[i]);
		}
	}
}
