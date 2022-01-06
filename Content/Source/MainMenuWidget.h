#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "MainMenuWidget.generated.h"

USTRUCT(BlueprintType)
struct FSaveSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Index;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FDateTime DateTime;
};

UCLASS()
class SURVIVALFPS_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FSaveSlot> SaveSlots;

public:
	bool Toggle();

	void UpdateSlot(const FString& SaveName, int Index, const FDateTime& DateTime);

	virtual bool Initialize() override;
};
