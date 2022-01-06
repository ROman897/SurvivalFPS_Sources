// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerHUD.h"
#include "Engine/Canvas.h"
#include "TextureResource.h"
#include "CanvasItem.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Texture2D.h"
#include "PlayerCharacter.h"
#include <Kismet/GameplayStatics.h>
#include "CharacterEquipment.h"
#include "ToolComponent.h"
#include "GunComponent.h"

APlayerHUD::APlayerHUD()
{
}

/** This method draws a very simple crosshair */
void APlayerHUD::DrawHUD()
{
	Super::DrawHUD();

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetOwningPawn());
	if (PlayerCharacter == nullptr) {
		return;
	}

	if (PlayerCharacter->IsSprinting()) {
		return;
	}

	UToolComponent* PrimaryToolComp = PlayerCharacter->GetPrimaryToolComponent();
	UGunComponent* PrimaryGunComp = Cast<UGunComponent>(PrimaryToolComp);

	if (PrimaryGunComp == nullptr) {
		return;
	}

	float AngleInaccuracy = PrimaryGunComp->CalculateAngleInaccuracy();

	FVector MuzzleLocation = PrimaryGunComp->GetMuzzleLocation();
	FVector ShootDirection = PrimaryGunComp->GetFireDirection() * CrosshairDistanceFromMuzzle;

	FVector2D CrosshairRightScreenLocation = RotateAroundAndProject(MuzzleLocation, ShootDirection, AngleInaccuracy, PrimaryGunComp->GetUpDirection());
	FVector2D CrosshairCenterScreenLocation = RotateAroundAndProject(MuzzleLocation, ShootDirection, 0.0f, PrimaryGunComp->GetUpDirection());
	float SideDistFromCenter = (CrosshairCenterScreenLocation - CrosshairRightScreenLocation).Size();

	CrosshairRightScreenLocation = CrosshairCenterScreenLocation + FVector2D(SideDistFromCenter, 0.0f);
	FVector2D CrosshairLeftScreenLocation = CrosshairCenterScreenLocation + FVector2D(-SideDistFromCenter, 0.0f);

	FVector2D CrosshairTopScreenLocation = CrosshairCenterScreenLocation + FVector2D(0.0f, -SideDistFromCenter);
	FVector2D CrosshairBotScreenLocation = CrosshairCenterScreenLocation + FVector2D(0.0f, SideDistFromCenter);

	FVector2D LeftPos = CrosshairLeftScreenLocation + FVector2D(-CrosshairLineLength, -CrosshairLineWidth / 2);
	FVector2D RightPos = CrosshairRightScreenLocation + FVector2D(0, -CrosshairLineWidth / 2);
	FVector2D TopPos = CrosshairTopScreenLocation + FVector2D(-CrosshairLineWidth / 2, -CrosshairLineLength);
	FVector2D BotPos = CrosshairBotScreenLocation + FVector2D(-CrosshairLineWidth / 2, 0);

	FVector2D LinePositions[4] = { LeftPos, RightPos, TopPos, BotPos };
	FVector2D LineSizes[4] = {
		FVector2D(CrosshairLineLength, CrosshairLineWidth),
		FVector2D(CrosshairLineLength, CrosshairLineWidth),
		FVector2D(CrosshairLineWidth, CrosshairLineLength),
		FVector2D(CrosshairLineWidth, CrosshairLineLength)
	};

	for (int i = 0; i < 4; ++i) {
		// Draw the crosshair
		FCanvasTileItem TileItem(LinePositions[i], CrosshairTex->Resource, LineSizes[i], FLinearColor::White);
		TileItem.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem( TileItem );
	}
}

FVector2D APlayerHUD::RotateAroundAndProject(FVector Origin, FVector Direction, float Angle, FVector Axis) const
{
	FVector RotatedDirection = Direction.RotateAngleAxis(Angle, Axis);
	FVector RotatedLocation = Origin + RotatedDirection;
	FVector2D ProjectedLocation;
	UGameplayStatics::ProjectWorldToScreen(GetOwningPlayerController(), RotatedLocation, ProjectedLocation);
	return ProjectedLocation;
}
