// Fill out your copyright notice in the Description page of Project Settings.


#include "AICharacter.h"
#include "GunComponent.h"

#include <Perception/AIPerceptionComponent.h>
#include <Kismet/GameplayStatics.h>
#include <GameFramework/CharacterMovementComponent.h>
#include <Blueprint/AIBlueprintHelperLibrary.h>
#include <BehaviorTree/BlackboardComponent.h>
#include <AIController.h>
#include <Math/UnrealMathUtility.h>
#include <Perception/AISense.h>
#include <Perception/AISense_Sight.h>
#include <Perception/AISense_Hearing.h>

#include "PlayerCharacter.h"
#include "Components/Mercuna3DNavigationComponent.h"
#include "Components/MercunaGroundNavigationComponent.h"
#include "Mercuna3DMovement/Public/Mercuna3DMovementComponent.h"

AAICharacter::AAICharacter()
{
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));

	GunComponent = CreateDefaultSubobject<UGunComponent>(TEXT("GunComp"));
	GunComponent->SetOwner(this);
	PrimaryTool = GunComponent;

	GunMuzzle = CreateDefaultSubobject<USceneComponent>(TEXT("GunMuzzleComp"));
	GunMuzzle->SetupAttachment(GetMesh(), FName("GunMuzzle"));
	GunComponent->GunMuzzle = GunMuzzle;

	MercunaGroundNavigationComponent = CreateDefaultSubobject<UMercunaGroundNavigationComponent>(TEXT("MercunaGroundNavigationComp"));

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

ETeamAttitude::Type AAICharacter::GetTeamAttitudeTowards(const AActor& Other) const
{
	AAIController* AIController = Cast<AAIController>(GetController());
	return AIController ? AIController->GetTeamAttitudeTowards(Other) : ETeamAttitude::Type::Neutral;
}

void AAICharacter::BeginFiring()
{
	PrimaryToolFirePressed();
}

void AAICharacter::StopFiring()
{
	PrimaryToolFireReleased();
}

void AAICharacter::BeginCombat()
{
	bInCombat = true;
}

void AAICharacter::StopCombat()
{
	bInCombat = false;
	StopFiring();
}

bool AAICharacter::TrySetNewTarget()
{
	TArray<ABasicCharacter*> VisibleEnemiesArr;
	for (const auto& VisibleEnemy : VisibleEnemies) {
		VisibleEnemiesArr.Add(VisibleEnemy);
	}

	if (VisibleEnemiesArr.Num() > 0) {
		ABasicCharacter* NewTarget;

		if (VisibleEnemiesArr.Num() > 1) {
			NewTarget = VisibleEnemiesArr[FMath::RandRange(0, VisibleEnemies.Num() - 1)];
		}
		else {
			NewTarget = VisibleEnemiesArr[0];
		}
		Blackboard->SetValueAsEnum(FName("TargetVisibility"), static_cast<uint8>(ETargetVisibility::VISIBLE));
		Blackboard->SetValueAsObject(FName("CurrentTarget"), NewTarget);
		CurrentTargetCharacter = NewTarget;

		return true;
		UE_LOG(LogTemp, Warning, TEXT("setting new targeto"));
	}
	UE_LOG(LogTemp, Warning, TEXT("no targeto found"));
	return false;
}

FVector AAICharacter::GetTargetLastKnownPosition()
{
	auto LastStimulus = StimuliByCharacter.Find(CurrentTargetCharacter);
	if (LastStimulus != nullptr) {
		return LastStimulus->StimulusLocation;
	}
	check(false);
	// TODO(Roman): what do we do in this branch?!
	return FVector();
}

TArray<FAIStimulus> AAICharacter::ReadLatestStimuli()
{
	// TODO(Roman): not sure how blueprints treat rvalue, so just move it to tmp variable
	// and return that by value for now

	TArray<FAIStimulus> TmpStimuli = std::move(StimuliByTime);
	Blackboard->SetValueAsInt(FName("StimuliCount"), 0);
	return TmpStimuli;
}

void AAICharacter::FocusTarget()
{
	Cast<AAIController>(GetController())->SetFocus(CurrentTargetCharacter);
}

void AAICharacter::ClearFocus()
{
	AAIController* AIController = Cast<AAIController>(GetController());
	check(AIController);
	AIController->ClearFocus(0);
	UE_LOG(LogTemp, Warning, TEXT("clear focuso"));
}

void AAICharacter::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AAICharacter::TargetPerceptionUpdated);
		Blackboard = UAIBlueprintHelperLibrary::GetBlackboard(this);
		check(Blackboard != nullptr);
		Blackboard->SetValueAsEnum(FName("AIState"), static_cast<uint8>(EAIState::IDLE));
		Blackboard->SetValueAsEnum(FName("AIAction"), static_cast<uint8>(EAIAction::IDLE));
	}
}

void AAICharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		if (bDead) {
			return;
		}

		if (CurrentTargetCharacter != nullptr) {
			float Dist = (CurrentTargetCharacter->GetActorLocation() - GetActorLocation()).SizeSquared2D();
			Blackboard->SetValueAsFloat(FName("DistanceToTargetSquared"), Dist);
		}
	}
}

void AAICharacter::TargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	APlayerCharacter* PerceptedCharacter = Cast<APlayerCharacter>(Actor);
	if (PerceptedCharacter == nullptr) {
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("perceptioo"));

	if (!PerceptedCharacter->IsAlive()) {
		return;
		// TODO(Roman): test how perception works if you delete player character

		//EnemiesInRange.Remove(PerceptedCharacter);
		//Visibility = EObservedCharacterVisibility::INVALID;
	}
	else {
		// if ai character sees enemy
		if (UAISense::GetSenseID<UAISense_Sight>() == Stimulus.Type) {
			if (Stimulus.WasSuccessfullySensed()) {
				VisibleEnemies.Add(PerceptedCharacter);
			}
			else {
				VisibleEnemies.Remove(PerceptedCharacter);
				if (CurrentTargetCharacter == PerceptedCharacter) {

					Blackboard->SetValueAsEnum(FName("TargetVisibility"), static_cast<uint8>(ETargetVisibility::INVISIBLE));
				}
				UE_LOG(LogTemp, Warning, TEXT("target loost"));
			}
			Blackboard->SetValueAsInt(FName("VisibleEnemiesCount"), VisibleEnemies.Num());
		}
		StimuliByCharacter.Emplace(PerceptedCharacter, Stimulus);
		StimuliByTime.Emplace(Stimulus);
		Blackboard->SetValueAsInt(FName("StimuliCount"), StimuliByTime.Num());
	}
}
