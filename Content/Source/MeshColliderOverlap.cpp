// Fill out your copyright notice in the Description page of Project Settings.


#include "MeshColliderOverlap.h"


// Sets default values
AMeshColliderOverlap::AMeshColliderOverlap()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	StaticMesh->SetMobility(EComponentMobility::Movable);
	StaticMesh->SetCollisionProfileName(FName("OverlapAllDynamic"));
	StaticMesh->SetGenerateOverlapEvents(true);
	StaticMesh->OnComponentBeginOverlap.AddDynamic(this, &AMeshColliderOverlap::OnBeginOverlap);
	SetRootComponent(StaticMesh);
}

UStaticMeshComponent* AMeshColliderOverlap::GetStaticMeshComponent()
{
	return StaticMesh;
}

// Called when the game starts or when spawned
void AMeshColliderOverlap::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMeshColliderOverlap::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Overlapped with %s: "), *OtherActor->GetName());
}

// Called every frame
void AMeshColliderOverlap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

