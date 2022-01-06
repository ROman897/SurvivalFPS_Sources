// Fill out your copyright notice in the Description page of Project Settings.


#include "AIAffilController.h"
#include "InGamePlayerController.h"

FGenericTeamId AAIAffilController::GetGenericTeamId() const
{
	return GenericTeamId;
}

ETeamAttitude::Type AAIAffilController::GetTeamAttitudeTowards(const AActor& Other) const
{
	const APawn* OtherPawn = Cast<APawn>(&Other);

	// we don't expect to see anything other than a pawn here
	check(OtherPawn);

	ETeamAffil OtherAffil = ETeamAffil::NEUTRAL;

	const AController* OtherController = OtherPawn->GetController();
	const IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>(OtherController);
	if (TeamAgentInterface != nullptr) {
		OtherAffil = static_cast<ETeamAffil>(TeamAgentInterface->GetGenericTeamId().GetId());
	}

	check(OtherAffil == ETeamAffil::PLAYER || OtherAffil == ETeamAffil::NEUTRAL || OtherAffil == ETeamAffil::ENEMY);

	if (OtherAffil == TeamAffil) {
		return ETeamAttitude::Type::Friendly;
	}
	if (TeamAffil == ETeamAffil::NEUTRAL || OtherAffil == ETeamAffil::NEUTRAL) {
		return ETeamAttitude::Type::Neutral;
	}
	return ETeamAttitude::Type::Hostile;
}

void AAIAffilController::BeginPlay()
{
	Super::BeginPlay();
	GenericTeamId = FGenericTeamId(static_cast<uint8>(TeamAffil));
}
