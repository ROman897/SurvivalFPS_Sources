// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPart.h"


// Sets default values
AShipPart::AShipPart()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRootComp"));
	SetRootComponent(SceneRoot);
}