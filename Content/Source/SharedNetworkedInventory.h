// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NetworkedInventoryBase.h"
#include "SharedNetworkedInventory.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API USharedNetworkedInventory : public UNetworkedInventoryBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly)
		bool bAllPlayersHaveUI;
	
protected:
	void BeginPlay() override;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
