// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <Templates/Tuple.h>
#include "BasicCharacter.h"

#include <GenericTeamAgentInterface.h>

#include "Ship.h"
#include "ShipData.h"
#include "VoxelEnums.h"
#include "VoxelIntBox.h"
#include "WorldManager.h"
#include "Items/Generator.h"

#include "PlayerCharacter.generated.h"

class UInputComponent;
class UCameraComponent;
class USkeletalMeshComponent;
class USoundBase;
class UAnimMontage;

enum class EPlayerMode {
	NONE,
	BUILDING,
	DECONSTRUCTING,
	DIGGING,
	FLATTENING,
	CONTINUOUS_INTERACTION
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FDrawTerrainCircleDelegate, const FVector&, CenterLocation, float, Radius, float, Falloff, EVoxelFalloff, VoxelFalloff);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHideTerrainCircleDelegate);

UCLASS(config=Game)
class APlayerCharacter : public ABasicCharacter
{
	GENERATED_BODY()

private:
	friend class UCharacterEquipment;

	static constexpr int BUILDING_CHOICES = 1;
	static constexpr float BUILDING_POS_SNAP = 10.0f;
	static constexpr float BUILDING_ROT_SNAP = 45.0f;

	FGenerator Generator;

protected:

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraStabilizer* CameraStabilizer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UPrivateNetworkedInventory* NetworkedBagInventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USharedNetworkedInventory* NetworkedEquipmentInventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UCharacterEquipment* CharacterEquipment;

	class UShipPartDataAsset* CurrentBuildingDataAsset;

	UPROPERTY(EditAnywhere)
	float BuildingPosSnap;

	UPROPERTY(EditAnywhere)
	float BuildingRotSnap;

	UPROPERTY(EditAnywhere)
	float BuildingPreviewDefaultDistance;

	UPROPERTY(EditAnywhere)
	float MaxBuildDistance;

	UPROPERTY(EditAnywhere)
	float MinBuildDistance;

	UPROPERTY(EditAnywhere)
	float MouseWheelSpeed;

	class AShipPartPreviewBase* CurrentBuildingPreview = nullptr;

	bool bNewPreview = true;
	float CurrentPreviewDistance;

	int32 CurrentPreviewZRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UAIPerceptionStimuliSourceComponent* AIPerceptionStimuli;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UInventoryWidget> InventoryWidgetClass;

	// must remain UPROPERTY in order to survive garbage collection when not in viewport
	UPROPERTY()
	class UInventoryWidget* InventoryWidget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UCraftingListWidget> CraftingListWidgetClass;

	// must remain UPROPERTY in order to survive garbage collection when not in viewport
	UPROPERTY()
	class UCraftingListWidget* CraftingListWidget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UBuildMenuWidget> BuildMenuWidgetClass;

	// must remain UPROPERTY in order to survive garbage collection when not in viewport
	UPROPERTY()
	class UBuildMenuWidget* BuildMenuWidget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UMainMenuWidget> MainMenuWidgetClass;

	// must remain UPROPERTY in order to survive garbage collection when not in viewport
	UPROPERTY()
	class UMainMenuWidget* MainMenuWidget;

	EPlayerMode CurrentPlayerMode = EPlayerMode::NONE;

	class IInteractableInterface* FocusedInteractable = nullptr;
	static constexpr float MAX_INTERACT_DISTANCE = 1000.0f;

	UPROPERTY(VisibleAnywhere)
	class UPlayerPropsReplacer* PlayerPropsReplacer;

	UPROPERTY(EditAnywhere)
	float DigStrength;

	UPROPERTY(EditAnywhere)
	float DigRadius;

	UPROPERTY(EditAnywhere)
	float DigFalloff;

	UPROPERTY(EditAnywhere)
	float MaxDigDistance;

	UPROPERTY(EditAnywhere)
	float DigSphereTraceRadius;

	float CurrentDigCooldown = 0.0f;

	UPROPERTY(EditAnywhere)
	float DigCooldown = 0.0f;

	UPROPERTY(EditAnywhere)
	float MaxFlattenDistance;

	UPROPERTY(EditAnywhere)
	float FlattenStrength;

	UPROPERTY(EditAnywhere)
	float FlattenRadius;

	UPROPERTY(EditAnywhere)
	float FlattenFalloff;

	UPROPERTY(EditAnywhere)
	float FlattenSphereTraceRadius;

	bool bFlattening;
	FVector FlattenStartLocation;
	
	bool bTerrainCircleIsDrawn;

	FVector GroundHitLocation;

	TArray<FVoxelIntBox> ModifiedVoxelBoxes;

	UPROPERTY(BlueprintAssignable)
	FDrawTerrainCircleDelegate DrawTerrainCircleDelegate;

	UPROPERTY(BlueprintAssignable)
	FHideTerrainCircleDelegate HideTerrainCircleDelegate;

	UPROPERTY(EditAnywhere)
	bool bDebugTerrain;

	UPROPERTY(EditAnywhere)
	class UEquipableItemDataAsset* ControlGunItemDataAsset;

	UPROPERTY()
	class AShipControls* CurrentShipControls;

	UPROPERTY(VisibleAnywhere)
	class UChildActorComponent* GrapplingHookChildActorComponent;

	class AGrapplingHook* GrapplingHook;

	UPROPERTY(VisibleAnywhere)
	UCapsuleComponent* AttachToShipRangeCapsuleComponent;

	class IInteractableInterface* CurrentContinuousInteraction;

	float CurrentContinuousInteractionTimer;
	float CurrentContinuousInteractionCooldown;

	UPROPERTY(EditAnywhere)
	float DeconstructRange;

	UPROPERTY(EditAnywhere)
	float DeconstructTime;

	FShipPartHandle DeconstructShipPartHandle;

	UPROPERTY()
	class AShip* DeconstructHitShip;
	
	bool bDeconstructHitValidShipPart = false;
	float CurrentDeconstructTime = 0.0f;

	UPROPERTY()
	AShip* AttachedToThisShip = nullptr;

	UPROPERTY()
	class UShipCharacterMovementComponent* ShipCharacterMovementComponent;

	UPROPERTY(VisibleAnywhere)
	class UParticleSystemComponent* NearbyFogParticleSystem;

public:
	APlayerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void Freeze();
	void Unfreeze();

	virtual void ToolFired() override;

	int AddItemToBag(const struct FItemInstanceStack& ItemStack);

	class UNetworkedInventoryBase* GetNetworkedBagInventory();
	class UNetworkedInventoryBase* GetNetworkedEquipmentInventory();
	class UCharacterEquipment* GetCharacterEquipment();

	void InitializeLocallyControlled();
	void InitializeAuthority();

	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	UFUNCTION(BlueprintCallable)
	void CmdCraftItem(class UComposedItemDataAsset* ItemDataAsset, int Count);

	void BuildShipActorPartCmd(class AShip* Ship, FVector Location, FRotator Rotation);

	void BuildFoundationShipPartCmd(class AShip* Ship,
		const FSerializedShipFoundationPartData& SerializedShipFoundationPartData);

	UFUNCTION(BlueprintCallable)
	UUserWidget* GetCraftingListWidget();

	UFUNCTION(BlueprintCallable)
	UUserWidget* GetBuildMenuWidget();

	UFUNCTION(BlueprintCallable)
	UUserWidget* GetInventoryWidget();

	UFUNCTION(BlueprintCallable)
	UUserWidget* GetMainMenuWidget();

	UFUNCTION(BlueprintCallable)
	void SaveGame(const FString& SaveName, int Index, UPARAM(ref) FDateTime& DateTime);

	UFUNCTION(BlueprintCallable)
	void LoadGame(const FString& SaveName, int Index);
	
	void SetCurrentShipControls(class AShipControls* ShipControls);

protected:
	virtual void Restart() override;
	
	void AsyncSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccess);
	void AsyncLoadComplete(const FString& SlotName, const int32 UserIndex, class USaveGame* LoadedData);

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	// methods called from keyboard input
	void OnLeftMouseClicked();
	void OnLeftMouseReleased();
	void OnRightMouseClicked();
	void OnMouseWheel(float Val);
	void OnRotateLeft();
	void OnRotateRight();
	void OnRollLeft();
	void OnRollRight();
	void OnStartSprint();
	void OnReleaseSprint();

	void LeftMousePressedInBuildMode();
	void ReloadCmd();

	UFUNCTION(BlueprintCallable)
	void InteractCmd();

	UFUNCTION(BlueprintCallable)
	void InteractCmdReleased();

	void DeconstructCmd();
	void DeconstructCmdReleased();

	void HandleContinuousInteraction(float DeltaTime);

	void FireGrapplingHook();

	void DigCmdPressed();
	void DigCmdReleased();
	void Dig();

	UFUNCTION(Server, Reliable)
	void ServerRPCAddTerrainModification(const FTerrainModification& TerrainModification);
	void ServerRPCAddTerrainModification_Implementation(const FTerrainModification& TerrainModification);
	
	void InitiateFlattenCmd();
	void LeftMousePressedInFlattenMode();
	void Flatten(FVector FlattenLocation);

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles strafing movement, left and right */
	void MoveRight(float Val);

	void ShipPitch(float Val);

	UFUNCTION(BlueprintCallable)
	void StartBlueprintBuild(class UShipPartDataAsset* BuildingDataAsset);

	TPair<FVector, int32> GetPreviewTransform();
	float RoundCoord(float coord);
	float RoundAngle(float angle);

	void UpdateFocusedInteractable();
	void ClearFocusedInteractable();
	void SetFocusedInteractable(class IInteractableInterface* NewFocusedInteractable);

	void UpdateBuildingPreview();

	FHitResult IntersectTerrain(float Range);
	FHitResult SphereIntersectTerrain(float Range, float Radius, FVector& OutHitSphereCenter);

	void DeconstructTick(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	class AWorldManager* GetWorldManager();
	
	UFUNCTION()
	void AttachToShipRangeColliderBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
	void AttachToShipRangeColliderEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	void UpdateShipAttachment();

	void UpdateNearbyFogParticleSystemVisibility();

private:
	UFUNCTION(Server, Reliable)
	void ServerRPCBuildShipActorPart(class UShipPartDataAsset* ShipPartDataAsset, class AShip* Ship, FVector Location, FRotator Rotation);
	void ServerRPCBuildShipActorPart_Implementation(class UShipPartDataAsset* ShipPartDataAsset, class AShip* Ship, FVector Location, FRotator Rotation);
	
	UFUNCTION(Server, Reliable)
	void ServerRPCBuildFoundationShipPart(AShip* Ship,
		const FSerializedShipFoundationPartData& SerializedShipFoundationPartData);
	void ServerRPCBuildFoundationShipPart_Implementation(AShip* Ship,
		const FSerializedShipFoundationPartData& SerializedShipFoundationPartData);
	
	UFUNCTION(Server, Reliable)
	void ServerRPCSetSprint(bool Value);
	void ServerRPCSetSprint_Implementation(bool Value);

	UFUNCTION(Server, Reliable)
	void ServerRPCCraftItem(class UComposedItemDataAsset* ItemDataAsset, int Count);
	void ServerRPCCraftItem_Implementation(class UComposedItemDataAsset* ItemDataAsset, int Count);
};
