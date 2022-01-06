// Fill out your copyright notice in the Description page of Project Settings.


#include "InGamePlayerController.h"
#include "AIAffilController.h"
#include "PlayerCharacter.h"
#include <Blueprint/WidgetBlueprintLibrary.h>

#include "DebugPrinter.h"
#include "WorldManager.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

AInGamePlayerController::AInGamePlayerController()
{
	FadeCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FadeCameraComp"));
	FadeCameraComponent->SetupAttachment(GetRootComponent());
}

void AInGamePlayerController::CharacterPossessed(APlayerCharacter* PlayerCharacter)
{
	/*AWorldManager* WorldInstance = AWorldManager::GetInstance();
	check(WorldInstance != nullptr);

	if (IsLocalController())
	{
		APlayerCameraManager* CameraManager = Cast<APlayerCameraManager>(UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0));
		CameraManager->SetManualCameraFade(1.0f, FColor::Black, false);
	}
	
	if (!WorldInstance->IsReady())
	{
		WorldInstance->WorldReadyDelegate.AddDynamic(this, &AInGamePlayerController::OnWorldReady);

		GetPawn<APlayerCharacter>()->Freeze();
	} else
	{
		OnWorldReady();
	}*/
}

void AInGamePlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AInGamePlayerController::OnWorldReady()
{
	FString NetMode = UDebugPrinter::GetPIENetMode(GetWorld());
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: controller %s OnWorldReady called with authority"), *NetMode, *GetName());
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: controller %s OnWorldReady called without authority"), *NetMode, *GetName());
	}

	APlayerCharacter* PlayerCharacter = GetPawn<APlayerCharacter>();
	check(PlayerCharacter != nullptr);

	if (HasAuthority())
	{
		FHitResult GroundHit = AWorldManager::GetInstance()->LineTraceGround(FVector2D::ZeroVector);
		check(GroundHit.Actor != nullptr);
		PlayerCharacter->InitializeAuthority();
	}

	PlayerCharacter->InitializeLocallyControlled();
	PlayerCharacter->Unfreeze();

	if (IsLocalController())
	{
		GetWorld()->GetTimerManager().SetTimer(FadeDelayHandle, this, &AInGamePlayerController::FadeIn, BeginFadeDelay * 0.5f);
	}
}

void AInGamePlayerController::FadeIn()
{
	APlayerCameraManager* CameraManager = Cast<APlayerCameraManager>(UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0));
	CameraManager->StartCameraFade(1.0f, 0.0f, BeginFadeInDuration, FColor::Black);
	
	GetWorld()->GetTimerManager().SetTimer(HUDSpawnDelayHandle, this, &AInGamePlayerController::SpawnHUD, BeginFadeInDuration);
}

void AInGamePlayerController::SpawnHUD()
{
	ClientSetHUD(PlayerHUDClass);
}

FGenericTeamId AInGamePlayerController::GetGenericTeamId() const
{
	return FGenericTeamId(static_cast<uint8>(ETeamAffil::PLAYER));
}

void AInGamePlayerController::TryToggleInGameWidget(UUserWidget* FocusedWidget)
{
	if (CurrentPlayerInteraction == ECurrentPlayerInteraction::NONE) {
		OpenInGameWidget(FocusedWidget);
	}
	else {
		if (CurrentPlayerInteraction == ECurrentPlayerInteraction::UI_WIDGET) {
			CloseInGameWidget();
		}
	}
}

void AInGamePlayerController::OpenInGameWidget(UUserWidget* FocusedWidget)
{
	if (CurrentPlayerInteraction != ECurrentPlayerInteraction::NONE) {
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("opening in game widget"));
	CurrentPlayerInteraction = ECurrentPlayerInteraction::UI_WIDGET;
	CurrentInGameWidget = FocusedWidget;
	FocusedWidget->AddToViewport();
	
	// TODO(Roman): crashed here, investigate why
	GetPawn<APlayerCharacter>()->DisableInput(this);
	UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(this, FocusedWidget, EMouseLockMode::LockAlways, false);
	bShowMouseCursor = true;
}

void AInGamePlayerController::CloseInGameWidget()
{
	check(CurrentInGameWidget);
	CurrentPlayerInteraction = ECurrentPlayerInteraction::NONE;
	CurrentInGameWidget->RemoveFromParent();

	// TODO(Roman): crashed here, investigate why
	GetPawn<APlayerCharacter>()->EnableInput(this);
	
	UWidgetBlueprintLibrary::SetInputMode_GameOnly(this);
	bShowMouseCursor = false;
}
