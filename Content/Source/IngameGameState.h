// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/GameStateBase.h"
#include "IngameGameState.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API AIngameGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	TArray<class AShip*> AllPlayerShips;
};
