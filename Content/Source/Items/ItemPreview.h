#pragma once

#include "CoreMinimal.h"

#include "ItemInstance.h"
#include "Blueprint/UserWidget.h"

#include "ItemPreview.generated.h"

/**
*
*/
UCLASS(Blueprintable)
class SURVIVALFPS_API UItemPreview : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	TArray<FString> GetDescription(const FItemInstance& ItemInstance);
};
