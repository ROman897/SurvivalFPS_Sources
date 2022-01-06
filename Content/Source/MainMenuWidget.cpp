#include "MainMenuWidget.h"

#include "SurvivalFPSSaveGame.h"

void UMainMenuWidget::UpdateSlot(const FString& SaveName, int Index, const FDateTime& DateTime)
{
	FSaveSlot& SaveSlot = SaveSlots[Index];
	SaveSlot.Name = SaveName;
	SaveSlot.Index = Index;
	SaveSlot.DateTime = DateTime;
}

bool UMainMenuWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}
	// TODO(peto) make this configurable or do something with it
	const size_t Size = 5;
	SaveSlots.SetNum(Size);
	TArray<FString> SaveGameFiles = USurvivalFPSSaveGame::GetSaveGameFiles();
	check(SaveSlots.Num() >= SaveGameFiles.Num());
	for (const FString& SaveGameFile : SaveGameFiles)
	{
		FSaveSlot SaveSlot;
		USurvivalFPSSaveGame::FileToSaveGameFileInfo(SaveGameFile, SaveSlot.Name, SaveSlot.Index, SaveSlot.DateTime);
		SaveSlots[SaveSlot.Index] = SaveSlot;
	}
	for (size_t Idx = 0; Idx < Size; ++Idx)
	{
		SaveSlots[Idx].Index = Idx;
	}
	return true;
}

bool UMainMenuWidget::Toggle() {
	if (IsInViewport()) {
		RemoveFromParent();
		return false;
	}
	AddToViewport();
	return true;
}
