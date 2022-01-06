// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameMode.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API AMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
		void LoadGameMapWithSeed(int32 Seed);
	
	UFUNCTION(BlueprintCallable)
		void LoadGameMapRandSeed();
};
