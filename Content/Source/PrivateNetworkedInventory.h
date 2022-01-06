// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NetworkedInventoryBase.h"
#include "PrivateNetworkedInventory.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API UPrivateNetworkedInventory : public UNetworkedInventoryBase
{
	GENERATED_BODY()
	
protected:
	void BeginPlay() override;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
