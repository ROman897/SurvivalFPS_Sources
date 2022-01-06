// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Items/ItemInstance.h"

#include "SurvivalFPSSaveGame.generated.h"

/**
 *
 */
UCLASS()
class SURVIVALFPS_API USurvivalFPSSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// State of Inventory
	UPROPERTY()
	TArray<FItemInstanceStack> InventoryState;

	// Player level
	UPROPERTY()
	int PlayerLevel;

	// Exp needed to reach next level
	UPROPERTY()
	int ExpToNextLevel;

	// Name of the player
	UPROPERTY()
	FString PlayerName;

	USurvivalFPSSaveGame();

	static TArray<FString> GetSaveGameFiles();

	static void FileToSaveGameFileInfo(const FString& SaveGameFile, FString& Name, int& Index, FDateTime& LastModification);

	static FString CreateSaveGameFilename(const FString& Name, int Index);
};
