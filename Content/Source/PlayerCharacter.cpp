// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerCharacter.h"
#include "PrivateNetworkedInventory.h"
#include "SharedNetworkedInventory.h"
#include "NetworkedInventoryBase.h"
#include "CharacterEquipment.h"
#include "CraftingListWidget.h"
#include "BuildMenuWidget.h"
#include "InteractableInterface.h"
#include "InventoryWidget.h"
#include "InGamePlayerController.h"
#include "BuildingSplineBase.h"
#include "MainMenuWidget.h"
#include "Items/Assets/EquipableItemDataAsset.h"

#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include <GameFramework/CharacterMovementComponent.h>
#include <Runtime/Engine/Classes/Kismet/KismetMathLibrary.h>
#include <Net/UnrealNetwork.h>
#include <Perception/AIPerceptionStimuliSourceComponent.h>
#include <Perception/AISense_Hearing.h>
#include <Components/SphereComponent.h>

#include "ShipCharacterMovementComponent.h"
#include "WorldManager.h"
#include "Items/Generator.h"
#include "Items/Assets/ShipPartDataAsset.h"
#include "VoxelTools/VoxelSurfaceTools.h"

#include "CameraStabilizer.h"
#include "CollisionTraceChannels.h"
#include "GrapplingHook.h"
#include "PlayerPropsReplacer.h"
#include "Ship.h"
#include "ShipControls.h"
#include "SurvivalFPSSaveGame.h"
#include "Particles/ParticleSystemComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogPlayerChar, Warning, All);

#define COLLISION_INTERACT ECC_GameTraceChannel3
#define GROUND_TRACE_CHANNEL ECC_GameTraceChannel6

//////////////////////////////////////////////////////////////////////////
// APlayerCharacter

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass(ACharacter::CharacterMovementComponentName, UShipCharacterMovementComponent::StaticClass()))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	CameraStabilizer = CreateDefaultSubobject<UCameraStabilizer>(TEXT("CameraStabilizerComp"));
	CameraStabilizer->SetupAttachment(GetMesh(), FName("FP_Camera"));

	// Create a CameraComponent
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetRootComponent());

	CameraStabilizer->SetCamera(FirstPersonCameraComponent);
	
	NetworkedBagInventory = CreateDefaultSubobject<UPrivateNetworkedInventory>(TEXT("NetworkedBagInventoryComponent"));
	NetworkedEquipmentInventory = CreateDefaultSubobject<USharedNetworkedInventory>(TEXT("NetworkedEquipmentInventoryComponent"));
	CharacterEquipment = CreateDefaultSubobject<UCharacterEquipment>(TEXT("CharacterEquipmentComponent"));

	AIPerceptionStimuli = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("AIPerceptionStimuliComp"));

	PlayerPropsReplacer = CreateDefaultSubobject<UPlayerPropsReplacer>(TEXT("PlayerPropsReplacerComp"));
	PlayerPropsReplacer->SetupAttachment(GetRootComponent());

	GrapplingHookChildActorComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("GrapplingHookChildActorComp"));
	GrapplingHookChildActorComponent->SetupAttachment(GetRootComponent());

	AttachToShipRangeCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("AttachToShipRangeCapsuleComp"));
	AttachToShipRangeCapsuleComponent->SetupAttachment(GetRootComponent());
	AttachToShipRangeCapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::AttachToShipRangeColliderBeginOverlap);
	AttachToShipRangeCapsuleComponent->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::AttachToShipRangeColliderEndOverlap);

	ShipCharacterMovementComponent = Cast<UShipCharacterMovementComponent>(GetCharacterMovement());
	
	NearbyFogParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("NearbyFogParticleSystemComp"));
	NearbyFogParticleSystem->SetupAttachment(GetRootComponent());
}

void APlayerCharacter::Freeze()
{
	GetCharacterMovement()->SetActive(false);
}

void APlayerCharacter::Unfreeze()
{
	GetCharacterMovement()->SetActive(true);
}

void APlayerCharacter::ToolFired()
{
	Super::ToolFired();
	UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), 1.0f, this);
}

//////////////////////////////////////////////////////////////////////////
// Input

void APlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up gameplay key bindings

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APlayerCharacter::OnLeftMouseClicked);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &APlayerCharacter::OnLeftMouseReleased);
	PlayerInputComponent->BindAction("FireRight", IE_Pressed, this, &APlayerCharacter::OnRightMouseClicked);
	PlayerInputComponent->BindAxis("MouseWheel", this, &APlayerCharacter::OnMouseWheel);
	PlayerInputComponent->BindAction("RotateLeft", IE_Pressed, this, &APlayerCharacter::OnRotateLeft);
	PlayerInputComponent->BindAction("RotateRight", IE_Pressed, this, &APlayerCharacter::OnRotateRight);
	PlayerInputComponent->BindAction("RollLeft", IE_Pressed, this, &APlayerCharacter::OnRollLeft);
	PlayerInputComponent->BindAction("RollRight", IE_Pressed, this, &APlayerCharacter::OnRollRight);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APlayerCharacter::OnStartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlayerCharacter::OnReleaseSprint);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &APlayerCharacter::ReloadCmd);
	PlayerInputComponent->BindAction("Dig", IE_Pressed, this, &APlayerCharacter::DigCmdPressed);
	PlayerInputComponent->BindAction("Dig", IE_Released, this, &APlayerCharacter::DigCmdReleased);
	PlayerInputComponent->BindAction("Flatten", IE_Pressed, this, &APlayerCharacter::InitiateFlattenCmd);

	//PlayerInputComponent->BindAction("Inventory", IE_Pressed, this, &APlayerCharacter::ToggleInventory);
	PlayerInputComponent->BindAction("FireGrapplingHook", IE_Pressed, this, &APlayerCharacter::FireGrapplingHook);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis("ShipPitch", this, &APlayerCharacter::ShipPitch);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Deconstruct", IE_Pressed, this, &APlayerCharacter::DeconstructCmd);
	PlayerInputComponent->BindAction("Deconstruct", IE_Released, this, &APlayerCharacter::DeconstructCmdReleased);

	//for (int i = 0; i <= BUILDING_CHOICES; ++i) {
	//	PlayerInputComponent->BindAction(FName(FString("Build") + FString::FromInt(i)), IE_Pressed, this, &APlayerCharacter::BlueprintBuild);
	//}
}

TPair<FVector, int32> APlayerCharacter::GetPreviewTransform()
{
	TPair<FVector, int32> res;

	FVector ViewPointLocation;
	FRotator ViewPointRotation;
	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	check(PlayerController != nullptr);

	PlayerController->GetPlayerViewPoint(ViewPointLocation, ViewPointRotation);
	if (bNewPreview) {
		bNewPreview = false;
		CurrentPreviewDistance = BuildingPreviewDefaultDistance;
		//CurrentPreviewRotation = FRotator(0.0f, RoundAngle(ViewPointRotation.Yaw), 0.0f);
		CurrentPreviewZRotation = 0;
	}

	FVector UnroundedPos = ViewPointLocation + ViewPointRotation.Vector() * CurrentPreviewDistance;
	res.Get<0>() = FVector(RoundCoord(UnroundedPos.X), RoundCoord(UnroundedPos.Y), RoundCoord(UnroundedPos.Z));
	res.Get<1>() = CurrentPreviewZRotation;
	return res;
}

float APlayerCharacter::RoundCoord(float coord)
{
	float Divided = coord / BuildingPosSnap;
	return UKismetMathLibrary::Round(Divided) * BuildingPosSnap;
}

float APlayerCharacter::RoundAngle(float angle)
{
	float Divided = angle / BuildingRotSnap;
	return UKismetMathLibrary::Round(Divided) * BuildingRotSnap;
}

void APlayerCharacter::UpdateFocusedInteractable()
{
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(InteractTrace), true, this);

	FHitResult Hit(ForceInit);
	FVector StartTrace = FirstPersonCameraComponent->GetComponentLocation();
	FVector EndTrace = StartTrace + FirstPersonCameraComponent->GetForwardVector() * MAX_INTERACT_DISTANCE;
	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, COLLISION_INTERACT, TraceParams);

	AActor* HitActor = Hit.GetActor();
	IInteractableInterface* NewInteractable = Cast<IInteractableInterface>(HitActor);

	if (FocusedInteractable != nullptr) {
		if (NewInteractable != FocusedInteractable) {
			ClearFocusedInteractable();
			if (NewInteractable != nullptr) {
				SetFocusedInteractable(NewInteractable);
			}
		}
	}
	else {
		if (NewInteractable != nullptr) {
			SetFocusedInteractable(NewInteractable);
		}
	}
}

void APlayerCharacter::ClearFocusedInteractable()
{
	if (FocusedInteractable != nullptr)
	{
		FocusedInteractable->SetIsFocused(false);
		FocusedInteractable = nullptr;
	}

}

void APlayerCharacter::SetFocusedInteractable(IInteractableInterface* NewFocusedInteractable)
{
	FocusedInteractable = NewFocusedInteractable;
	FocusedInteractable->SetIsFocused(true);
}

void APlayerCharacter::UpdateBuildingPreview()
{
	check(CurrentBuildingPreview);
	CurrentBuildingPreview->SetDesiredPlacement(FirstPersonCameraComponent->GetComponentLocation(), FirstPersonCameraComponent->GetForwardVector(), CurrentPreviewZRotation);
}

void APlayerCharacter::ServerRPCBuildShipActorPart_Implementation(UShipPartDataAsset* ShipPartDataAsset, AShip* Ship, FVector Location, FRotator Rotation)
{
	check(ShipPartDataAsset);

	if (!IsValid(Ship))
	{
		return;
	}
	
	// TODO(peto) maybe rework?
	TArray<FItemInstanceStack> RequiredResources;
	for (const FRequiredResource& RequiredResource : ShipPartDataAsset->RequiredResources)
	{
		RequiredResources.AddZeroed();
		RequiredResources.Last().ItemRef.ItemDataAsset = RequiredResource.Item;
		RequiredResources.Last().Count = RequiredResource.Count;
	}

	// TODO(Roman): rework stupid conversion between required resources and iteminstance
	// TODO(Roman): 1. check if can take items 2. check if can add ship part 3. take items from inventory
	if (NetworkedBagInventory->TryTakeItems(RequiredResources)) {
		Ship->AddShipActorPart(ShipPartDataAsset, Location, Rotation);
	}
}

void APlayerCharacter::ServerRPCBuildFoundationShipPart_Implementation(AShip* Ship,
	const FSerializedShipFoundationPartData& SerializedShipFoundationPartData)
{
	if (!IsValid(Ship))
	{
		return;
	}
	
	// TODO(peto) maybe rework?
	TArray<FItemInstanceStack> RequiredResources;
	for (const FRequiredResource& RequiredResource : SerializedShipFoundationPartData.ShipPartDataAsset->RequiredResources)
	{
		RequiredResources.AddZeroed();
		RequiredResources.Last().ItemRef.ItemDataAsset = RequiredResource.Item;
		RequiredResources.Last().Count = RequiredResource.Count;
	}

	if (!NetworkedBagInventory->CanTakeItems(RequiredResources))
	{
		return;
	}
	if (Ship->AddFoundationShipPart(SerializedShipFoundationPartData))
	{
		NetworkedBagInventory->TakeItems(RequiredResources);
	}
}

void APlayerCharacter::ServerRPCSetSprint_Implementation(bool Value)
{
	bSprint = Value;
	if (bSprint) {
		GetCharacterMovement()->MaxWalkSpeed = MaxSprintSpeed;
	}
	else {
		GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeed;
	}
}

void APlayerCharacter::ServerRPCCraftItem_Implementation(UComposedItemDataAsset* ItemDataAsset, int Count)
{
	TArray<FItemInstanceStack> RequiredResources;
	for (const FRequiredResource& RequiredResource : ItemDataAsset->RequiredResources)
	{
		RequiredResources.AddZeroed();
		RequiredResources.Last().ItemRef.ItemDataAsset = RequiredResource.Item;
		// TODO(peto) validate
		RequiredResources.Last().Count = RequiredResource.Count * Count;
	}
	FItemInstanceStack NewItem;
	NewItem.ItemRef.ItemDataAsset = ItemDataAsset;
	NewItem.Count = Count;
	if (NetworkedBagInventory->TryTakeItems(RequiredResources)) {
		AddItemToBag(NewItem);
	}
}

void APlayerCharacter::OnLeftMouseClicked()
{
	if (bDebugTerrain)
	{
		FHitResult GroundHit = IntersectTerrain(2000);
		if (Cast<AVoxelWorld>(GroundHit.Actor) != nullptr)
		{
			AWorldManager::GetInstance()->DebugCalculateBiomesAtLocation(GroundHit.Location);
		}
	}
	
	switch (CurrentPlayerMode)
	{
		case EPlayerMode::BUILDING:
			LeftMousePressedInBuildMode();
			break;
		case EPlayerMode::FLATTENING:
			LeftMousePressedInFlattenMode();
			break;
		case EPlayerMode::NONE:
			PrimaryToolFirePressed();
			break;
	}
}

void APlayerCharacter::OnLeftMouseReleased()
{
	switch (CurrentPlayerMode)
	{
		case EPlayerMode::NONE:
		PrimaryToolFireReleased();
			break;
		case EPlayerMode::FLATTENING:
			bFlattening = false;
			AWorldManager::GetInstance()->RecalculateNavigationInVoxelBoxes(ModifiedVoxelBoxes);
			ModifiedVoxelBoxes.Empty();
			break;
	}
}

void APlayerCharacter::OnRightMouseClicked()
{
	if (CurrentBuildingPreview != nullptr) {
		CurrentBuildingPreview->Destroy();
		CurrentBuildingPreview = nullptr;
		bNewPreview = true;
		CurrentPlayerMode = EPlayerMode::NONE;
	}
}

void APlayerCharacter::OnMouseWheel(float Val)
{
	switch (CurrentPlayerMode)
	{
		case EPlayerMode::BUILDING:
			CurrentBuildingPreview->OnMouseWheel(Val);
			break;
		case EPlayerMode::NONE:
			if (AnimInstance->Montage_IsPlaying(nullptr)) {
				break;
			}
			if (Val > 0.0f) {
				CharacterEquipment->SwitchToolUp();
			}
			else {
				if (Val < 0.0f) {
					CharacterEquipment->SwitchToolDown();
				}
			}
			break;
	}
}

void APlayerCharacter::OnRotateLeft()
{
	CurrentPreviewZRotation--;
}

void APlayerCharacter::OnRotateRight()
{
	CurrentPreviewZRotation++;
}

void APlayerCharacter::OnRollLeft()
{
	//CurrentPreviewRotation.Pitch -= BuildingRotSnap;
}

void APlayerCharacter::OnRollRight()
{
	//CurrentPreviewRotation.Pitch += BuildingRotSnap;
}

void APlayerCharacter::OnStartSprint()
{
	ServerRPCSetSprint(true);
}

void APlayerCharacter::OnReleaseSprint()
{
	ServerRPCSetSprint(false);
}

int APlayerCharacter::AddItemToBag(const FItemInstanceStack& ItemStack)
{
	return NetworkedBagInventory->AddItem(ItemStack.ItemRef, ItemStack.Count);
}

UNetworkedInventoryBase* APlayerCharacter::GetNetworkedBagInventory()
{
	return NetworkedBagInventory;
}

UNetworkedInventoryBase* APlayerCharacter::GetNetworkedEquipmentInventory()
{
	return NetworkedEquipmentInventory;
}

UCharacterEquipment* APlayerCharacter::GetCharacterEquipment()
{
	return CharacterEquipment;
}

void APlayerCharacter::InitializeLocallyControlled()
{
	CraftingListWidget = CreateWidget<UCraftingListWidget>(UGameplayStatics::GetPlayerController(GetWorld(), 0), CraftingListWidgetClass, FName("PlayerCrafingList"));
	check(CraftingListWidget);
	
	BuildMenuWidget = CreateWidget<UBuildMenuWidget>(UGameplayStatics::GetPlayerController(GetWorld(), 0), BuildMenuWidgetClass, FName("BuildMenu"));
	check(BuildMenuWidget);
	
	MainMenuWidget = CreateWidget<UMainMenuWidget>(UGameplayStatics::GetPlayerController(GetWorld(), 0), MainMenuWidgetClass, FName("MainMenu"));
	check(MainMenuWidget);
	
	InventoryWidget = CreateWidget<UInventoryWidget>(UGameplayStatics::GetPlayerController(GetWorld(), 0), InventoryWidgetClass, FName("PlayerInventory"));
	check(InventoryWidget);
	InventoryWidget->SetLinkedNetworkedInventories({ NetworkedBagInventory, NetworkedEquipmentInventory });

	//AWorldManager::GetInstance()->RegisterPlayer(this);
}

void APlayerCharacter::InitializeAuthority()
{
	FItemInstanceStack NewItem;
	NewItem.ItemRef = Generator.GenerateResource("IronOre");
	NewItem.Count = 12;
	AddItemToBag(NewItem);
	FItemInstanceStack NewItem2;
	NewItem2.ItemRef = Generator.GenerateResource("BronzeIngot", EItemRarity::Common, true);
	NewItem2.Count = 42;
	AddItemToBag(NewItem2);
	FItemInstanceStack NewItem3;
	NewItem3.ItemRef = Generator.GenerateResource("Wood");
	NewItem3.Count = 24;
	AddItemToBag(NewItem3);
	FItemInstanceStack NewItem4;
	NewItem4.ItemRef = Generator.GenerateConsumable("Potion", 5);
	NewItem4.Count = 5;
	AddItemToBag(NewItem4);
	FItemInstanceStack NewItem5;
	NewItem5.ItemRef = Generator.GenerateWeapon("Rifle", 10, EItemRarity::Legendary);
	NewItem5.Count = 1;
	AddItemToBag(NewItem5);
	FItemInstanceStack NewItem6;
	NewItem6.ItemRef = Generator.GenerateEquip("Shield", 15, EItemRarity::Legendary);
	NewItem6.Count = 1;
	AddItemToBag(NewItem6);
}

void APlayerCharacter::LeftMousePressedInBuildMode()
{
	//auto SpawnLocRot = GetPreviewTransform();
	//ServerRPCBuild(CurrentBuildingDataAsset, SpawnLocRot.Get<0>(), SpawnLocRot.Get<1>());
	CurrentBuildingPreview->LeftMouseClicked(this);
}

void APlayerCharacter::ReloadCmd()
{
	TryReload();
}

void APlayerCharacter::InteractCmd()
{
	if (FocusedInteractable == nullptr) {
		return;
	}
	if (CurrentShipControls != nullptr)
	{
		CurrentShipControls->PlayerControlCancelled();
		CurrentShipControls = nullptr;
		return;
	}

	if (FocusedInteractable->IsContinuousInteraction())
	{
		CurrentPlayerMode = EPlayerMode::CONTINUOUS_INTERACTION;
		CurrentContinuousInteractionTimer = 0.0f;
		CurrentContinuousInteraction = FocusedInteractable;
		CurrentContinuousInteractionCooldown = CurrentContinuousInteraction->GetContinuousInteractionCooldown();
		
	} else
	{
		FocusedInteractable->Interact(this);
	}
}

void APlayerCharacter::InteractCmdReleased()
{
	CurrentContinuousInteraction = nullptr;
	if (CurrentPlayerMode == EPlayerMode::CONTINUOUS_INTERACTION)
	{
		CurrentPlayerMode = EPlayerMode::NONE;
	}
}

void APlayerCharacter::DeconstructCmd()
{
	if (CurrentPlayerMode == EPlayerMode::NONE)
	{
		CurrentPlayerMode = EPlayerMode::DECONSTRUCTING;
	}
}

void APlayerCharacter::DeconstructCmdReleased()
{
	if (CurrentPlayerMode == EPlayerMode::DECONSTRUCTING)
	{
		CurrentPlayerMode = EPlayerMode::NONE;
	}
}

void APlayerCharacter::HandleContinuousInteraction(float DeltaTime)
{
	CurrentContinuousInteractionTimer += DeltaTime;
	if (CurrentContinuousInteractionTimer >= CurrentContinuousInteractionCooldown)
	{
		CurrentContinuousInteractionTimer -= CurrentContinuousInteractionCooldown;
		CurrentContinuousInteraction->Interact(this);
	}
}

void APlayerCharacter::FireGrapplingHook()
{
	FVector TraceStart = FirstPersonCameraComponent->GetComponentLocation();
	FVector TraceEnd = TraceStart + FirstPersonCameraComponent->GetForwardVector() * GrapplingHook->GetMaxRange();

	FHitResult GrapplingHookHit;
	if (GetWorld()->LineTraceSingleByChannel(GrapplingHookHit, TraceStart, TraceEnd, TRACE_CHANNEL_SOLID_MATTER))
	{
		GrapplingHook->Fire(GrapplingHookHit.Location);
	} else
	{
		GrapplingHook->Fire(TraceEnd);
	}
}

void APlayerCharacter::DigCmdPressed()
{
	if (CurrentPlayerMode != EPlayerMode::NONE)
	{
		return;
	}

	CurrentPlayerMode = EPlayerMode::DIGGING;
	CurrentDigCooldown = 0.0f;
}

void APlayerCharacter::DigCmdReleased()
{
	CurrentPlayerMode = EPlayerMode::NONE;
}

void APlayerCharacter::Dig()
{
	FVector HitSphereLocation;
	FHitResult GroundHit = SphereIntersectTerrain(MaxDigDistance, DigSphereTraceRadius, HitSphereLocation);
	
	if (Cast<AVoxelWorld>(GroundHit.Actor) == nullptr)
	{
		return;
	}
	FTerrainModification TerrainModification {HitSphereLocation, FVector(), DigStrength, DigRadius, DigFalloff, EVoxelFalloff::Linear, ETerrainModificationType::DIG};
	ServerRPCAddTerrainModification(TerrainModification);
}

void APlayerCharacter::ServerRPCAddTerrainModification_Implementation(const FTerrainModification& TerrainModification)
{
	AWorldManager::GetInstance()->AddTerrainModification(TerrainModification);
}

void APlayerCharacter::InitiateFlattenCmd()
{
	if (CurrentPlayerMode == EPlayerMode::NONE)
	{
		CurrentPlayerMode = EPlayerMode::FLATTENING;
		//FlattenTerrainPreviewActor = GetWorld()->SpawnActor(FlattenTerrainPreviewActorClass);
		//UpdateFlattenPreviewNotDragging();
	} else
	{
		if (CurrentPlayerMode == EPlayerMode::FLATTENING)
		{
			CurrentPlayerMode = EPlayerMode::NONE;
		}
	}
}

void APlayerCharacter::LeftMousePressedInFlattenMode()
{
	FHitResult GroundHit = IntersectTerrain(MaxFlattenDistance);
	if (Cast<AVoxelWorld>(GroundHit.Actor) == nullptr)
	{
		return;
	}

	FlattenStartLocation = GroundHit.Location;
	bFlattening = true;
}

void APlayerCharacter::Flatten(FVector FlattenLocation)
{
	FTerrainModification TerrainModification {FlattenLocation, FlattenStartLocation, FlattenStrength, FlattenRadius, FlattenFalloff, EVoxelFalloff::Linear, ETerrainModificationType::FLATTEN};
	ServerRPCAddTerrainModification(TerrainModification);
}

FHitResult APlayerCharacter::IntersectTerrain(float Range)
{
	FName TraceTag("DigTrace");
	//GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(DigTrace), true, this);

	FHitResult Hit(ForceInit);
	FVector StartTrace = FirstPersonCameraComponent->GetComponentLocation();
	FVector EndTrace = StartTrace + FirstPersonCameraComponent->GetForwardVector() * Range;
	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, GROUND_TRACE_CHANNEL, TraceParams);
	return Hit;
}

FHitResult APlayerCharacter::SphereIntersectTerrain(float Range, float Radius, FVector& OutHitSphereCenter)
{
	FName TraceTag("DigTrace");
	//GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(DigTrace), true, this);

	FHitResult Hit(ForceInit);
	FVector StartTrace = FirstPersonCameraComponent->GetComponentLocation();
	FVector EndTrace = StartTrace + FirstPersonCameraComponent->GetForwardVector() * Range;

	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(Radius);
	GetWorld()->SweepSingleByChannel(Hit, StartTrace, EndTrace, FQuat::Identity, GROUND_TRACE_CHANNEL, CollisionShape, TraceParams);

	if (Hit.Actor != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("hit actor: %s  at location: %s"), *Hit.Actor->GetName(), *Hit.Location.ToString());
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("hit nothing"));
	}

	if (Cast<AVoxelWorld>(Hit.Actor) != nullptr)
	{
		FPlane OriginPlane = FPlane(StartTrace, FirstPersonCameraComponent->GetForwardVector());
		FVector IntersectionOnOriginPlane = FMath::RayPlaneIntersection(Hit.Location, -FirstPersonCameraComponent->GetForwardVector(), OriginPlane);
		FVector DiffOnOriginPlane = StartTrace - IntersectionOnOriginPlane;
		OutHitSphereCenter = Hit.Location + DiffOnOriginPlane;
	}
	return Hit;
}

void APlayerCharacter::DeconstructTick(float DeltaTime)
{
	FName TraceTag("DeconstructTrace");
	//GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(DeconstructTrace), false, this);

	FHitResult Hit(ForceInit);
	FVector StartTrace = FirstPersonCameraComponent->GetComponentLocation();
	FVector EndTrace = StartTrace + FirstPersonCameraComponent->GetForwardVector() * DeconstructRange;
	
	bool HitAnything = GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, TRACE_CHANNEL_SOLID_MATTER);
	if (!HitAnything)
	{
		// we didn't hit anything, so reset and return
		bDeconstructHitValidShipPart = false;
		return;
	}
	
	// we hit something

	AShip* HitShip = AShip::FindParentShip(Hit.GetActor());
	if (HitShip == nullptr) {
		// we didn't hit a ship
		bDeconstructHitValidShipPart = false;
		return;
	}
	
	FShipPartHandle HitShipPartHandle;

	if (!HitShip->GetShipPartHandleFromHit(Hit, HitShipPartHandle))
	{
		// we didn't hit a valid ship part
		bDeconstructHitValidShipPart = false;
		return;
	}

	if (bDeconstructHitValidShipPart && DeconstructShipPartHandle == HitShipPartHandle)
	{
		CurrentDeconstructTime += DeltaTime;
	} else
	{
		DeconstructHitShip = HitShip;
		DeconstructShipPartHandle = HitShipPartHandle;
		CurrentDeconstructTime = DeltaTime;
		bDeconstructHitValidShipPart = true;
	}

	if (CurrentDeconstructTime >= DeconstructTime)
	{
		DeconstructHitShip->DeconstructShipPart(DeconstructShipPartHandle);
		bDeconstructHitValidShipPart = false;
	}
}

AWorldManager* APlayerCharacter::GetWorldManager()
{
	return AWorldManager::GetInstance();
}

void APlayerCharacter::AttachToShipRangeColliderBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UpdateShipAttachment();
}

void APlayerCharacter::AttachToShipRangeColliderEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UpdateShipAttachment();
}

void APlayerCharacter::UpdateShipAttachment()
{
	TArray<AActor*> OverlappingActors;
	AttachToShipRangeCapsuleComponent->GetOverlappingActors(OverlappingActors);

	FVector CurrentAbsoluteVelocity = GetVelocity();
	if (AttachedToThisShip != nullptr)
	{
		CurrentAbsoluteVelocity += AttachedToThisShip->GetVelocity();
	}

	AShip* LandedOnThisShip = nullptr;

	for (AActor* OverlappingActor : OverlappingActors)
	{
		if (AShip* OverlappingShip = AShip::FindParentShip(OverlappingActor))
		{
			LandedOnThisShip = OverlappingShip;
			break;
		}
	}

	if (AttachedToThisShip == LandedOnThisShip || (AttachedToThisShip == nullptr && LandedOnThisShip == nullptr))
	{
		return;
	}

	if (LandedOnThisShip != nullptr)
	{
		AttachToActor(LandedOnThisShip, FAttachmentTransformRules::KeepWorldTransform);
		ShipCharacterMovementComponent->SetVelocity(CurrentAbsoluteVelocity - LandedOnThisShip->GetVelocity());
	} else
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		ShipCharacterMovementComponent->SetVelocity(CurrentAbsoluteVelocity);
	}
	if (LandedOnThisShip != nullptr)
	{
		MovementBaseUtility::AddTickDependency(ShipCharacterMovementComponent->GetPrimaryTickFunction(), LandedOnThisShip->GetMovingPrimitiveComponent());
	}
	if (AttachedToThisShip != nullptr)
	{
		MovementBaseUtility::AddTickDependency(ShipCharacterMovementComponent->GetPrimaryTickFunction(), AttachedToThisShip->GetMovingPrimitiveComponent());
		//MovementBaseUtility::RemoveTickDependency(ShipCharacterMovementComponent->GetPrimaryTickFunction(), AttachedToThisShip->GetMovingPrimitiveComponent());
		
	}
	AttachedToThisShip = LandedOnThisShip;

}

void APlayerCharacter::UpdateNearbyFogParticleSystemVisibility()
{
	if (AttachedToThisShip == nullptr)
	{
		NearbyFogParticleSystem->SetHiddenInGame(false);
		return;
	}
	NearbyFogParticleSystem->SetHiddenInGame(AttachedToThisShip->IsPointInside(GetActorLocation()));
}

void APlayerCharacter::SetCurrentShipControls(AShipControls* ShipControls)
{
	CurrentShipControls = ShipControls;
}

void APlayerCharacter::MoveForward(float Value)
{
	if (CurrentShipControls != nullptr)
	{
		CurrentShipControls->AddSpeed(Value);
		return;
	}
	
	if (Value != 0.0f)
	{
		// Add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
		UE_LOG(LogTemp, Warning, TEXT("movement forward input: %f"), Value);
	}
	LastRawMovementInput.X = Value;
}

void APlayerCharacter::MoveRight(float Value)
{
	if (CurrentShipControls != nullptr)
	{
		CurrentShipControls->AddSteering(Value);
		return;
	}
	
	if (Value != 0.0f)
	{
		// Add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
	LastRawMovementInput.Y = Value;
}

void APlayerCharacter::ShipPitch(float Val)
{
	if (CurrentShipControls != nullptr)
	{
		CurrentShipControls->AddPitch(Val);
	}
}

void APlayerCharacter::StartBlueprintBuild(UShipPartDataAsset* BuildingDataAsset)
{
	ClearFocusedInteractable();
	CurrentPlayerMode = EPlayerMode::BUILDING;
	CurrentBuildingDataAsset = BuildingDataAsset;

	CurrentBuildingPreview = GetWorld()->SpawnActor<AShipPartPreviewBase>(BuildingDataAsset->ShipPartPreviewClass);
	CurrentBuildingPreview->SetShipPartDataAsset(CurrentBuildingDataAsset);
	CurrentBuildingPreview->SetDesiredPlacement(FirstPersonCameraComponent->GetComponentLocation(), FirstPersonCameraComponent->GetForwardVector(), CurrentPreviewZRotation);
	GetController<AInGamePlayerController>()->CloseInGameWidget();
}

void APlayerCharacter::CmdCraftItem(UComposedItemDataAsset* ItemDataAsset, int Count)
{
	ServerRPCCraftItem(ItemDataAsset, Count);
}

void APlayerCharacter::BuildShipActorPartCmd(AShip* Ship, FVector Location, FRotator Rotation)
{
	ServerRPCBuildShipActorPart(CurrentBuildingDataAsset, Ship, Location, Rotation);
}

void APlayerCharacter::BuildFoundationShipPartCmd(AShip* Ship,
	const FSerializedShipFoundationPartData& SerializedShipFoundationPartData)
{
	ServerRPCBuildFoundationShipPart(Ship, SerializedShipFoundationPartData);
}

UUserWidget* APlayerCharacter::GetCraftingListWidget()
{
	return CraftingListWidget;
}

UUserWidget* APlayerCharacter::GetBuildMenuWidget()
{
	return BuildMenuWidget;
}

UUserWidget* APlayerCharacter::GetInventoryWidget()
{
	return InventoryWidget;
}

UUserWidget* APlayerCharacter::GetMainMenuWidget()
{
	return MainMenuWidget;
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateNearbyFogParticleSystemVisibility();

	bool ShouldDrawTerrainCircle = false;

	switch (CurrentPlayerMode) {
		case EPlayerMode::BUILDING:
			UpdateBuildingPreview();
			break;

		case EPlayerMode::NONE:
			UpdateFocusedInteractable();
			break;
		case EPlayerMode::DIGGING:
			if (CurrentDigCooldown <= 0.0f)
			{
				Dig();
				CurrentDigCooldown = DigCooldown;
			} else
			{
				CurrentDigCooldown -= DeltaSeconds;
			}
			break;
		case EPlayerMode::DECONSTRUCTING:
			DeconstructTick(DeltaSeconds);
			break;
/*		case EPlayerMode::FLATTENING:

			FVector HitSphereLocation;
			FHitResult GroundHit = SphereIntersectTerrain(MaxFlattenDistance, FlattenSphereTraceRadius, HitSphereLocation);
			if (Cast<AVoxelWorld>(GroundHit.Actor) == nullptr)
			{
				ShouldDrawTerrainCircle = false;
				break;
			}
			ShouldDrawTerrainCircle = true;

			GroundHitLocation = HitSphereLocation;

			if (bFlattening)
			{
				Flatten(GroundHitLocation);
			}

			break;
			*/
		case EPlayerMode::CONTINUOUS_INTERACTION:
			HandleContinuousInteraction(DeltaSeconds);
			break;
	}

	if (ShouldDrawTerrainCircle)
	{
		bTerrainCircleIsDrawn = true;
		DrawTerrainCircleDelegate.Broadcast(GroundHitLocation, FlattenRadius, FlattenFalloff, EVoxelFalloff::Linear);
	} else
	{
		if (bTerrainCircleIsDrawn)
		{
			HideTerrainCircleDelegate.Broadcast();
		}
	}

	// player animations rotate head around X axis, so we have to set each frame
	// camera roll to 0 in world coords
	if (IsLocallyControlled()) {
		FRotator OldRotation = FirstPersonCameraComponent->GetComponentRotation();
		FRotator NewRotation = FRotator(OldRotation.Pitch, OldRotation.Yaw, 0.0f);
		FirstPersonCameraComponent->SetWorldRotation(NewRotation.Quaternion());
	}

	/*if (StandingOnMovingActor != nullptr)
	{
		FVector StandingOnMovingActorNewLocation = StandingOnMovingActor->GetActorLocation();
		SetActorLocation(GetActorLocation() + StandingOnMovingActorNewLocation - StandingOnMovingActorPrevPos);
		StandingOnMovingActorPrevPos = StandingOnMovingActorNewLocation;
		
	}*/
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	FItemInstance EquipableControlGunInstance {ControlGunItemDataAsset};
	NetworkedBagInventory->AddItem(EquipableControlGunInstance, 1);

	if (HasAuthority())
	{
		InitializeAuthority();
	}
	if (IsLocallyControlled())
	{
		InitializeLocallyControlled();
	}

	GrapplingHook = Cast<AGrapplingHook>(GrapplingHookChildActorComponent->GetChildActor());
	check(GrapplingHook != nullptr);
}

void APlayerCharacter::AsyncSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
	// TODO(peto) print info about save complete
	UE_LOG(LogTemp, Warning, TEXT("Save for %s on index: %d %s!"), *SlotName, UserIndex, bSuccess ? TEXT("done") : TEXT("failed"));
}

void APlayerCharacter::SaveGame(const FString& SaveName, int Index, FDateTime& DateTime)
{
	USurvivalFPSSaveGame* SaveGameInstance = Cast<USurvivalFPSSaveGame>(UGameplayStatics::CreateSaveGameObject(USurvivalFPSSaveGame::StaticClass()));
	check(SaveGameInstance != nullptr);

	FAsyncSaveGameToSlotDelegate SavedDelegate;
	SavedDelegate.BindUObject(this, &APlayerCharacter::AsyncSaveComplete);

	SaveGameInstance->InventoryState = NetworkedBagInventory->GetItems();
	SaveGameInstance->PlayerLevel = 1;
	SaveGameInstance->ExpToNextLevel = 0;
	SaveGameInstance->PlayerName = "TestName";
	DateTime = FDateTime::Now();

	FString FileName = USurvivalFPSSaveGame::CreateSaveGameFilename(SaveName, Index);
	UGameplayStatics::AsyncSaveGameToSlot(SaveGameInstance, FileName, 0, SavedDelegate);
	MainMenuWidget->UpdateSlot(SaveName, Index, DateTime);
}

void APlayerCharacter::AsyncLoadComplete(const FString& SaveName, const int32 Index, USaveGame* LoadedData)
{
	// TODO(peto) print info about load complete
	(void) SaveName;
	(void) Index;
	USurvivalFPSSaveGame* LoadGameInstance = Cast<USurvivalFPSSaveGame>(LoadedData);
	check(LoadGameInstance != nullptr);
	for (const FItemInstanceStack& SavedInventoryItems : LoadGameInstance->InventoryState)
	{
		if (SavedInventoryItems.ItemRef.ItemDataAsset == nullptr)
		{
			break;
		}
		AddItemToBag(SavedInventoryItems);
	}
}

void APlayerCharacter::LoadGame(const FString& SaveName, int Index)
{
	FString FileName = USurvivalFPSSaveGame::CreateSaveGameFilename(SaveName, Index);
	check(UGameplayStatics::DoesSaveGameExist(FileName, 0));

	FAsyncLoadGameFromSlotDelegate LoadedDelegate;
	LoadedDelegate.BindUObject(this, &APlayerCharacter::AsyncLoadComplete);
	UGameplayStatics::AsyncLoadGameFromSlot(FileName, 0, LoadedDelegate);
}

void APlayerCharacter::Restart()
{
	Super::Restart();
	GetController<AInGamePlayerController>()->CharacterPossessed(this);
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
