// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Engine/EngineTypes.h>

#include "Animation/WeaponAnimationData.h"
#include "Components/ActorComponent.h"

#include "ToolComponent.generated.h"

UENUM(BlueprintType)
enum class EToolPose : uint8 {
	NONE,
	RIFLE,
	PISTOL,
	PICKAXE,
	AXE
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIVALFPS_API UToolComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EToolPose ToolPose;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FWeaponAnimationData WeaponAnimationData;

	class ABasicCharacter* OwnerCharacter;

public:	
	// Sets default values for this component's properties
	UToolComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	FHitResult LineTrace(const FVector& StartTrace, const FVector& EndTrace, ECollisionChannel CollisionChannel, bool DrawTrace = false) const;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void FirePressed();
	virtual void FireReleased();

	virtual bool CanFire();
	virtual bool NeedReload();
	virtual bool CanReload();
	virtual void Reload();

	// returns true if tool is stateful, i.e. automatic firing rifle or cutting axe,
	// false for weapons such as sniper rifle that only fires once
	// must always return true for tools operated by AI
	virtual bool MustSendReleaseRPC();

	virtual void AnimNotify(int NotifyNumber);

	void SetOwner(class ABasicCharacter* OwnerChar);

	EToolPose GetToolPose();
	FWeaponAnimationData GetWeaponAnimationData();
	void SetMesh(UMeshComponent* MeshComponent);
};
