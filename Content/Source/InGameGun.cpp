// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameGun.h"
#include "GunComponent.h"

#include <Components/SkeletalMeshComponent.h>
#include <Components/SceneComponent.h>

// Sets default values
AInGameGun::AInGameGun()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GunMesh = CreateDefaultSubobject<USkeletalMeshComponent>("GunMeshComp");
	SetRootComponent(GunMesh);

	GunComponent = CreateDefaultSubobject<UGunComponent>("GunComp");
	ToolComponent = GunComponent;
	GunMuzzle = CreateDefaultSubobject<USceneComponent>("MuzzleLocationComp");
	GunMuzzle->SetupAttachment(GunMesh);
	GunComponent->GunMuzzle = GunMuzzle;

	GunComponent->SetMesh(GunMesh);
}
