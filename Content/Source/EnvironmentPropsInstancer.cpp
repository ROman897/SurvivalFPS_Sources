// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvironmentPropsInstancer.h"

#include "EnvironmentPropDataAsset.h"
#include "InstancedEnvironmentPropComponent.h"
#include "InstancedEnvironmentResourcePropComponent.h"
#include "PlayerCharacter.h"
#include "Components/BoxComponent.h"

AEnvironmentPropsInstancer* AEnvironmentPropsInstancer::Instance = nullptr;


// Sets default values
AEnvironmentPropsInstancer::AEnvironmentPropsInstancer()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AEnvironmentPropsInstancer::BeginPlay()
{
	Super::BeginPlay();
	Instance = this;
}

// Called every frame
void AEnvironmentPropsInstancer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

AEnvironmentPropsInstancer* AEnvironmentPropsInstancer::GetInstance()
{
	check(Instance != nullptr);
	return Instance;
}

void AEnvironmentPropsInstancer::RegisterEnvironmentProp(const UEnvironmentPropDataAsset* EnvironmentPropDataAsset)
{
	if (HierarchicalInstancedMeshComponents.Contains(EnvironmentPropDataAsset))
	{
		return;
	}

	FName Name = FName(FString("InstancedMesh") + EnvironmentPropDataAsset->GetName());
	
	UInstancedEnvironmentPropComponent* InstancedEnvironmentPropComponent;

	if (EnvironmentPropDataAsset->InGameResourceDataAsset != nullptr)
	{
		InstancedEnvironmentPropComponent = NewObject<UInstancedEnvironmentResourcePropComponent>(this, UInstancedEnvironmentResourcePropComponent::StaticClass(), Name);
		
	} else
	{
		InstancedEnvironmentPropComponent = NewObject<UInstancedEnvironmentPropComponent>(this, UInstancedEnvironmentPropComponent::StaticClass(), Name);
	}
	
	InstancedEnvironmentPropComponent->SetupAttachment(GetRootComponent());
	InstancedEnvironmentPropComponent->RegisterComponent();
	
	AddInstanceComponent(InstancedEnvironmentPropComponent);
	InstancedEnvironmentPropComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
	HierarchicalInstancedMeshComponents.Add(EnvironmentPropDataAsset, InstancedEnvironmentPropComponent);

	InstancedEnvironmentPropComponent->SetEnvironmentPropDataAsset(EnvironmentPropDataAsset);
}

void AEnvironmentPropsInstancer::AddInstance(const UEnvironmentPropDataAsset* EnvironmentPropDataAsset,
	const FTransform& Transform)
{
	auto HierarchicalInstancedMeshComponent = *HierarchicalInstancedMeshComponents.Find(EnvironmentPropDataAsset);
	check(HierarchicalInstancedMeshComponent);

	HierarchicalInstancedMeshComponent->AddInstance(Transform);
}
