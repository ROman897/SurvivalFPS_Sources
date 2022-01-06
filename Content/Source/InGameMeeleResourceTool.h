// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InGameTool.h"
#include "InGameMeeleResourceTool.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API AInGameMeeleResourceTool : public AInGameTool
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* ResourceToolMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UMeeleResourceToolComponent* MeeleResourceToolComponent;

public:
	AInGameMeeleResourceTool();
};
