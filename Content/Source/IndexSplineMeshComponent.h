// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Components/SplineMeshComponent.h"
#include "UObject/Object.h"
#include "IndexSplineMeshComponent.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API UIndexSplineMeshComponent : public USplineMeshComponent
{
	GENERATED_BODY()

public:
	int32 Id;
	bool bIsLast;
};
