// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include <GenericTeamAgentInterface.h>

#include "AIAffilController.generated.h"

UENUM(BlueprintType)
enum class ETeamAffil : uint8 {
	PLAYER,
	NEUTRAL,
	ENEMY
};

/**
 * 
 */
UCLASS()
class SURVIVALFPS_API AAIAffilController : public AAIController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	ETeamAffil TeamAffil;

private:
	FGenericTeamId GenericTeamId;

public:
	FGenericTeamId GetGenericTeamId() const override;
	ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

protected:
	void BeginPlay() override;
};
