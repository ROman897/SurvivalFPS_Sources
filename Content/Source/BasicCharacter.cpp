// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicCharacter.h"
#include "AbilityComponent.h"
#include "ToolComponent.h"

#include <Net/UnrealNetwork.h>
#include <GameFramework/CharacterMovementComponent.h>

#include "Components/MercunaObstacleComponent.h"

// Sets default values
ABasicCharacter::ABasicCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	MercunaObstacleComponent = CreateDefaultSubobject<UMercunaObstacleComponent>(TEXT("MercunaObstacleComp"));
	MercunaObstacleComponent->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void ABasicCharacter::BeginPlay()
{
	Super::BeginPlay();
	AnimInstance = GetMesh()->GetAnimInstance();
	CurrentHealth = 1.0f;
	GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeed;
	TArray<UActorComponent*> AbilityComponents;
	GetComponents(UAbilityComponent::StaticClass(), AbilityComponents, false);

	for (UActorComponent* AbilityComponent : AbilityComponents) {
		Abilities.Emplace(AbilityComponent->GetName(), Cast<UAbilityComponent>(AbilityComponent));
	}
}

void ABasicCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABasicCharacter, CurrentHealth);
	DOREPLIFETIME(ABasicCharacter, bSprint);
	DOREPLIFETIME_CONDITION(ABasicCharacter, Pitch, COND_SkipOwner);
}

void ABasicCharacter::Die()
{
	if (CorpseClass != nullptr) {
		GetWorld()->SpawnActor(CorpseClass, &GetTransform());
	}
	Destroy();
}

void ABasicCharacter::PrimaryToolFirePressed()
{
	if (!CanFirePrimaryTool()) {
		UE_LOG(LogTemp, Warning, TEXT("can't firee"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("should firee"));
	PrimaryTool->FirePressed();
	
	if (HasAuthority()) {
		MulticastRPCFirePressed();
	} else {
		ServerRPCFirePressed();
	}
}

void ABasicCharacter::PrimaryToolFireReleased()
{
	if (PrimaryTool != nullptr && PrimaryTool->MustSendReleaseRPC()) {
		PrimaryTool->FireReleased();
		if (HasAuthority()) {
			MulticastRPCFireReleased();
		}
		else {
			ServerRPCFireReleased();
		}
	}
}

bool ABasicCharacter::CanFirePrimaryTool()
{
	return PrimaryTool != nullptr && PrimaryTool->CanFire()
		&& !AnimInstance->Montage_IsPlaying(nullptr);
}

void ABasicCharacter::TryReload()
{
	if (!CanReload()) {
		return;
	}
	Reload();
}

// Called every frame
void ABasicCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (CanReload() && NeedReload()) {
		Reload();
	}
	UpdatePitch();
}

// Called to bind functionality to input
void ABasicCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ABasicCharacter::OnHit(float Damage)
{
	if (bDead) {
		return;
	}
	CurrentHealth -= Damage / MaxHp;
	bDead = CurrentHealth <= 0.0f;
	if (bDead) {
		Die();
	}
}

float ABasicCharacter::CalculateCharacterInaccuracy()
{
	return GetVelocity().Size() / GetMaxSpeed();
}

bool ABasicCharacter::IsSprinting()
{
	return bSprint;
}

float ABasicCharacter::GetMaxSpeed()
{
	return bSprint ? MaxSprintSpeed : MaxWalkSpeed;
}

bool ABasicCharacter::IsAlive()
{
	return !bDead;
}

bool ABasicCharacter::CanCastAbility()
{
	check(AnimInstance);
	return !AnimInstance->Montage_IsPlaying(nullptr);
}

void ABasicCharacter::PlayMontage(UAnimMontage* AnimMontage)
{
	AnimInstance->Montage_Play(AnimMontage);
}

UAbilityComponent* ABasicCharacter::GetAbilityByName(const FString& AbilityName)
{
	UAbilityComponent** Ability = Abilities.Find(AbilityName);
	if (Ability == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("basic character could not find ability: %s"), *AbilityName);
	}
	return Ability != nullptr ? *Ability : nullptr;
}

EToolPose ABasicCharacter::GetCurrentToolPose()
{
	return PrimaryTool != nullptr ? PrimaryTool->GetToolPose() : EToolPose::NONE;
}

void ABasicCharacter::ToolFired()
{
	check(AnimInstance);

	UAnimMontage** ToolFireMontage = ToolFireAnimations.Find(GetCurrentToolPose());
	if (ToolFireMontage != nullptr) {
		AnimInstance->Montage_Play(*ToolFireMontage);
	}
}

void ABasicCharacter::ServerRPCFirePressed_Implementation()
{
	if (CanFirePrimaryTool()) {
		MulticastRPCFirePressed();
	}
}

void ABasicCharacter::MulticastRPCFirePressed_Implementation()
{
	if (IsLocallyControlled()) {
		return;
	}
	if (CanFirePrimaryTool()) {
		PrimaryTool->FirePressed();
	}
}

void ABasicCharacter::ServerRPCFireReleased_Implementation()
{
	MulticastRPCFireReleased();
}

void ABasicCharacter::MulticastRPCFireReleased_Implementation()
{
	if (!IsLocallyControlled()) {
		if (PrimaryTool != nullptr) {
			PrimaryTool->FireReleased();
		}
	}
}

void ABasicCharacter::UpdatePitch()
{
	if (HasAuthority() || IsLocallyControlled()) {
		Pitch = GetControlRotation().Pitch;
	}
}

bool ABasicCharacter::CanReload()
{
	return PrimaryTool != nullptr && PrimaryTool->CanReload()
		&& !AnimInstance->Montage_IsPlaying(nullptr);
}

bool ABasicCharacter::NeedReload()
{
	return PrimaryTool->NeedReload();
}

void ABasicCharacter::Reload()
{
	check(AnimInstance);

	UAnimMontage** ReloadMontage = ToolReloadAnimations.Find(GetCurrentToolPose());
	if (ReloadMontage != nullptr) {
		AnimInstance->Montage_Play(*ReloadMontage);
	}
}

void ABasicCharacter::OnNotifyToolReloaded()
{
	check(PrimaryTool);
	PrimaryTool->Reload();
}

UToolComponent* ABasicCharacter::GetPrimaryToolComponent()
{
	return PrimaryTool;
}

FVector ABasicCharacter::GetLastRawMovementInput()
{
	return LastRawMovementInput;
}

FWeaponAnimationData ABasicCharacter::GetWeaponAnimationData()
{
	if (PrimaryTool != nullptr)
	{
		return PrimaryTool->GetWeaponAnimationData();
	}
	return FWeaponAnimationData();
}
