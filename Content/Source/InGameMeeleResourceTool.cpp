// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameMeeleResourceTool.h"
#include "MeeleResourceToolComponent.h"

#include <Components/StaticMeshComponent.h>

AInGameMeeleResourceTool::AInGameMeeleResourceTool()
{
	ResourceToolMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ResourceToolMeshComp"));
	SetRootComponent(ResourceToolMesh);
	
	MeeleResourceToolComponent = CreateDefaultSubobject<UMeeleResourceToolComponent>(TEXT("MeeleResourceToolComp"));
	ToolComponent = MeeleResourceToolComponent;
}
