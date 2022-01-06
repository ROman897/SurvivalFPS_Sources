// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityComponent.h"
#include "BasicCharacter.h"

// Sets default values for this component's properties
UAbilityComponent::UAbilityComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UAbilityComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = GetOwner<ABasicCharacter>();
	check(OwnerCharacter);

	// ...
	
}


// Called every frame
void UAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UAbilityComponent::UseAbility()
{
	if (OwnerCharacter->CanCastAbility()) {
		OwnerCharacter->PlayMontage(AbilityAnimation);
		return true;
	}
	return false;
}

void UAbilityComponent::AnimNotify(int NotifyNumber)
{
}
