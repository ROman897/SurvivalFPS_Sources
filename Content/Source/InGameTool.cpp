// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameTool.h"
#include "ToolComponent.h"

// Sets default values
AInGameTool::AInGameTool()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AInGameTool::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AInGameTool::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

UToolComponent& AInGameTool::GetToolComponent()
{
	return *ToolComponent;
}

