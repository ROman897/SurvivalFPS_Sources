// Fill out your copyright notice in the Description page of Project Settings.
#include "SurvivalFPSSaveGame.h"

#include "SurvivalFPS.h"

USurvivalFPSSaveGame::USurvivalFPSSaveGame()
{
	if (InventoryState.Num() > 0)
	{
		InventoryState.Empty();
	}

	PlayerLevel = 0;
	ExpToNextLevel = 0;
}

static FString SaveGamesPath {FString::Printf(TEXT("%sSaveGames/"), *FPaths::ProjectSavedDir())};

TArray<FString> USurvivalFPSSaveGame::GetSaveGameFiles()
{
	TArray<FString> SaveGameFiles;
	static FString SaveFilesExtension {"*.sav"};
	IFileManager::Get().FindFiles(SaveGameFiles, *SaveGamesPath, *SaveFilesExtension);
	return SaveGameFiles;
}

static FString PartsDelimiter {"_"};

void USurvivalFPSSaveGame::FileToSaveGameFileInfo(const FString& SaveGameFile, FString& Name, int& Index, FDateTime& LastModification)
{
	FString SaveGameFilename = FPaths::GetBaseFilename(SaveGameFile);
	TArray<FString> Parts;
	SaveGameFilename.ParseIntoArray(Parts, *PartsDelimiter);
	check(Parts.Num() == 2);
	Name = MoveTemp(Parts[0]);
	Index = FCString::Atoi(*Parts[1]);
	FString SaveGameFilePath {FString::Printf(TEXT("%s/%s"), *SaveGamesPath, *SaveGameFile)};
	LastModification = IFileManager::Get().GetTimeStamp(*SaveGameFilePath);
}

FString USurvivalFPSSaveGame::CreateSaveGameFilename(const FString& Name, int Index)
{
	return FString::Printf(
		TEXT("%s%c%d"),
		*Name,
		PartsDelimiter[0],
		Index
	);
}
