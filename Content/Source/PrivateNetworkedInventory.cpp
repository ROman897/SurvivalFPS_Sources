#include "PrivateNetworkedInventory.h"
#include "InGamePlayerController.h"

#include <Kismet/GameplayStatics.h>
#include <Net/UnrealNetwork.h>

void UPrivateNetworkedInventory::BeginPlay()
{
	Super::BeginPlay();
}

void UPrivateNetworkedInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UPrivateNetworkedInventory, Items, COND_OwnerOnly);
}
