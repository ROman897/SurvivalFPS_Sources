// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildMenuWidget.h"

#include "Items/Assets/ShipPartDataAsset.h"

bool UBuildMenuWidget::Toggle() {
	if (IsInViewport()) {
		RemoveFromParent();
		return false;
	}
	AddToViewport();
	return true;

}
