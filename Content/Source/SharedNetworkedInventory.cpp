#include "SharedNetworkedInventory.h"
#include "InGamePlayerController.h"

#include <Kismet/GameplayStatics.h>
#include <Net/UnrealNetwork.h>

void USharedNetworkedInventory::BeginPlay()
{
	Super::BeginPlay();
}

void USharedNetworkedInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USharedNetworkedInventory, Items);
}
