// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DebugPrinter.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API UDebugPrinter : public UObject
{
	GENERATED_BODY()

public:
	static FString GetPIENetMode(UWorld* WorldContext);
};
