// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"

UCLASS()
class APlayerHUD : public AHUD
{
	GENERATED_BODY()

public:
	APlayerHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

protected:
	UPROPERTY(EditAnywhere)
	float CrosshairDistanceFromMuzzle;

	UPROPERTY(EditAnywhere)
	class UTexture2D* CrosshairTex;

	UPROPERTY(EditAnywhere)
	float MinCrosshairSpace;

	UPROPERTY(EditAnywhere)
	float MaxCrosshairSpace;

	UPROPERTY(EditAnywhere)
	float CrosshairLineWidth;

	UPROPERTY(EditAnywhere)
	float CrosshairLineLength;

private:
	FVector2D RotateAroundAndProject(FVector Origin, FVector Direction, float Angle, FVector Axis) const;
};

