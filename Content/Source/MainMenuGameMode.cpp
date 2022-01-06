// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuGameMode.h"

#include "MyGameInstance.h"
#include "Kismet/GameplayStatics.h"

void AMainMenuGameMode::LoadGameMapWithSeed(int32 Seed)
{
	Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()))->SetSeed(Seed);
	GetWorld()->ServerTravel("GameMap", true);
}

void AMainMenuGameMode::LoadGameMapRandSeed() {
	Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()))->ResetSeed();
	GetWorld()->ServerTravel("GameMap", true);
}

