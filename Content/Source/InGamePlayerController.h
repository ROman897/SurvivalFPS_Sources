// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include <GenericTeamAgentInterface.h>
#include "InGamePlayerController.generated.h"

UENUM(BlueprintType)
enum class ECurrentPlayerInteraction : uint8
{
	NONE,
	UI_WIDGET
};

/**
 *
 */
UCLASS()
class SURVIVALFPS_API AInGamePlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly)
	ECurrentPlayerInteraction CurrentPlayerInteraction = ECurrentPlayerInteraction::NONE;

	class UUserWidget* CurrentInGameWidget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AHUD> PlayerHUDClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class APlayerCharacter> PlayerCharacterClass;

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* FadeCameraComponent;

	UPROPERTY(EditAnywhere)
	float BeginFadeDelay;

	UPROPERTY(EditAnywhere)
	float BeginFadeInDuration;

	FTimerHandle FadeDelayHandle;
	FTimerHandle HUDSpawnDelayHandle;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnWorldReady();

	UFUNCTION()
	void FadeIn();

	UFUNCTION()
	void SpawnHUD();

public:
	AInGamePlayerController();

	void CharacterPossessed(APlayerCharacter* PlayerCharacter);

	FGenericTeamId GetGenericTeamId() const override;

	UFUNCTION(BlueprintCallable)
	void TryToggleInGameWidget(UUserWidget* FocusedWidget);

	UFUNCTION(BlueprintCallable)
	void OpenInGameWidget(UUserWidget* FocusedWidget);

	// call when player e.g. closes the inventory
	UFUNCTION(BlueprintCallable)
	void CloseInGameWidget();
};
