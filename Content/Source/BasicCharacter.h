// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <Containers/Map.h>

#include "ToolComponent.h"
#include "VoxelCharacter.h"
#include "Animation/WeaponAnimationData.h"

#include "BasicCharacter.generated.h"

UCLASS()
class SURVIVALFPS_API ABasicCharacter : public AVoxelCharacter
{
	GENERATED_BODY()

protected:
	class UAnimInstance* AnimInstance;

	UPROPERTY(EditDefaultsOnly)
	float MaxHp;

	UPROPERTY(Replicated, BlueprintReadOnly)
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly)
	bool bDead = false;

	UPROPERTY(replicated)
	bool bSprint = false;

	UPROPERTY(EditAnywhere)
	float MaxSprintSpeed;

	UPROPERTY(EditAnywhere)
	float MaxWalkSpeed;

	UPROPERTY(Replicated, BlueprintReadOnly)
	float Pitch;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> CorpseClass;

	TMap<FString, class UAbilityComponent*> Abilities;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UToolComponent* PrimaryTool;

	UPROPERTY(EditAnywhere)
	TMap<EToolPose, UAnimMontage*> ToolFireAnimations;

	UPROPERTY(EditAnywhere)
	TMap<EToolPose, UAnimMontage*> ToolReloadAnimations;

	UPROPERTY(VisibleAnywhere)
	class UMercunaObstacleComponent* MercunaObstacleComponent;

	FVector LastRawMovementInput = FVector::ZeroVector;

public:
	// Sets default values for this character's properties
	ABasicCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void OnHit(float Damage);

	float CalculateCharacterInaccuracy();

	UFUNCTION(BlueprintCallable)
	bool IsSprinting();

	UFUNCTION(BlueprintCallable)
	float GetMaxSpeed();

	bool IsAlive();

	bool CanCastAbility();
	void PlayMontage(class UAnimMontage* AnimMontage);

	class UAbilityComponent* GetAbilityByName(const FString& AbilityName);

	UFUNCTION(BlueprintCallable)
	EToolPose GetCurrentToolPose();

	// base must be called!!
	virtual void ToolFired();
	void OnNotifyToolReloaded();

	UToolComponent* GetPrimaryToolComponent();

	FVector GetLastRawMovementInput();

	FWeaponAnimationData GetWeaponAnimationData();

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Die();

	UFUNCTION(BlueprintCallable)
	void PrimaryToolFirePressed();

	// only called on characters with authority
	UFUNCTION(BlueprintCallable)
	void PrimaryToolFireReleased();

	bool CanFirePrimaryTool();

	void TryReload();

	UFUNCTION(Server, Reliable)
	void ServerRPCFirePressed();
	void ServerRPCFirePressed_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCFirePressed();
	void MulticastRPCFirePressed_Implementation();

	UFUNCTION(Server, Reliable)
	void ServerRPCFireReleased();
	void ServerRPCFireReleased_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCFireReleased();
	void MulticastRPCFireReleased_Implementation();

private:
	void UpdatePitch();
	bool CanReload();
	bool NeedReload();
	void Reload();

};
