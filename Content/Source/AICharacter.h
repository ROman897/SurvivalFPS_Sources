// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasicCharacter.h"
#include <Perception/AIPerceptionTypes.h>
#include <Containers/Map.h>
#include <Containers/Array.h>
#include <GenericTeamAgentInterface.h>
#include "AICharacter.generated.h"

UENUM(BlueprintType)
enum class EAIState : uint8 {
	// enemy sees player and is actively attacking
	FIGHTING,

	// enemy does not see target, will start looking for them
	LOOKING_FOR_TARGET,

	// enemy is not in combat
	IDLE,

	INVESTIGATING_STIMULUS
};

UENUM(BlueprintType)
enum class EAIAction : uint8 {
	IDLE,
	DOING_ATTACK,
	MOVING
};

UENUM(BlueprintType)
enum class ETargetVisibility : uint8 {
	VISIBLE,
	INVISIBLE,
	INVALID
};

USTRUCT(BlueprintType)
struct FSenseObservation
{
	GENERATED_BODY()

	FVector LastKnownLocation;
};

UCLASS()
class SURVIVALFPS_API AAICharacter : public ABasicCharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	UAnimMontage* FireAnimation;

	UPROPERTY(VisibleAnywhere)
	class UAIPerceptionComponent* AIPerception;

	UPROPERTY(BlueprintReadOnly)
	bool bInCombat = false;

	ABasicCharacter* CurrentTargetCharacter;

	UPROPERTY(VisibleAnywhere)
	class USceneComponent* GunMuzzle;

	UPROPERTY(VisibleAnywhere)
	class UGunComponent* GunComponent;

	UPROPERTY(EditAnywhere)
	TArray<UAnimMontage*> SpecialAbilities;

	TMap<ABasicCharacter*, FAIStimulus> StimuliByCharacter;
	TArray<FAIStimulus> StimuliByTime;
	TSet<ABasicCharacter*> VisibleEnemies;

	class UBlackboardComponent* Blackboard;

	UPROPERTY(VisibleAnywhere)
	class UMercunaGroundNavigationComponent* MercunaGroundNavigationComponent;

protected:
	void BeginPlay() override;
	void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void TargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION(BlueprintCallable)
	void BeginFiring();

	UFUNCTION(BlueprintCallable)
	void StopFiring();

	UFUNCTION(BlueprintCallable)
	void BeginCombat();

	UFUNCTION(BlueprintCallable)
	void StopCombat();

	UFUNCTION(BlueprintCallable)
	bool TrySetNewTarget();

	UFUNCTION(BlueprintCallable)
	FVector GetTargetLastKnownPosition();

	UFUNCTION(BlueprintCallable)
	TArray<FAIStimulus> ReadLatestStimuli();

	UFUNCTION(BlueprintCallable)
	void FocusTarget();

	UFUNCTION(BlueprintCallable)
	void ClearFocus();

public:
	AAICharacter();

	ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

};
