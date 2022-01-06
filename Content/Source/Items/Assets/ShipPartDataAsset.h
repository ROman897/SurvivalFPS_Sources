#pragma once

#include "CoreMinimal.h"

#include "ComposedItemDataAsset.h"

// Note(Roman): we must include this because TSubclassOf apparently does not work with forward
// declaration when the class is inside parent directory
#include "../../ShipPartPreviewBase.h"
#include "SurvivalFPS/ShipData.h"

#include "ShipPartDataAsset.generated.h"

UCLASS(BlueprintType)
class SURVIVALFPS_API UShipPartDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString ShipPartName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UTexture2D* ShipPartIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FRequiredResource> RequiredResources;

	UPROPERTY(EditAnywhere)
	EShipPartTypes ShipPartType = EShipPartTypes::ACTOR;

	// only valid if ShipPartFoundationType == EShipPartTypes::FOUNDATION
	UPROPERTY(EditAnywhere)
	EShipPartSnapTypes ShipPartSnapType;

	// only valid if ShipPartFoundationType == EShipPartTypes::FOUNDATION
	UPROPERTY(EditAnywhere)
	UStaticMesh* ShipPartStaticMesh;

	// only valid if ShipPartFoundationType == EShipPartTypes::ACTOR
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AShipPart> ShipPartActorClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AShipPartPreviewBase> ShipPartPreviewClass;

	UPROPERTY(EditAnywhere)
	TArray<class UMaterialInterface*> Materials;
};
