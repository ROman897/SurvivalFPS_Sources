#pragma once

#include "CoreMinimal.h"

#include "InGamePlayerController.h"
#include "Components/StaticMeshComponent.h"

#include "PlayerCharacter.h"

#include "InteractableInterface.generated.h"

UINTERFACE(BlueprintType)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()

};

class IInteractableInterface
{
	GENERATED_BODY()
protected:
	void SetMeshFocused(UPrimitiveComponent* PrimitiveComponent, bool IsFocused);

	void OpenPlayerWidget(APlayerCharacter* InteractingPlayer, UUserWidget* PlayerWidget);

public:
	virtual void SetIsFocused(bool IsFocused) = 0;
	virtual void Interact(class APlayerCharacter* InteractingPlayer) = 0;
	virtual bool IsContinuousInteraction()
	{
		return false;
	}
	virtual float GetContinuousInteractionCooldown()
	{
		check(false);
		return 0;
	}
};

inline void IInteractableInterface::SetMeshFocused(UPrimitiveComponent* PrimitiveComponent, bool IsFocused)
{
	PrimitiveComponent->SetRenderCustomDepth(IsFocused);
	if (IsFocused) {
		PrimitiveComponent->SetCustomDepthStencilValue(2);
	}
}

inline void IInteractableInterface::OpenPlayerWidget(APlayerCharacter* InteractingPlayer, UUserWidget* PlayerWidget)
{
	AInGamePlayerController* PlayerController = InteractingPlayer->GetController<AInGamePlayerController>();
	PlayerController->OpenInGameWidget(PlayerWidget);
}
