// Fill out your copyright notice in the Description page of Project Settings.


#include "GrapplingHookCable.h"

#include "CableComponent.h"


// Sets default values
AGrapplingHookCable::AGrapplingHookCable()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CableComponent = CreateDefaultSubobject<UCableComponent>(TEXT("CableComp"));
	SetRootComponent(CableComponent);
}