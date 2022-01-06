// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PowerProducer.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UPowerProducer : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SURVIVALFPS_API IPowerProducer
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual float GetAvailablePower() = 0;
	virtual void TakePower(float Amount) = 0;
};
