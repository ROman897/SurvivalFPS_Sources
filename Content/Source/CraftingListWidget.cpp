// Fill out your copyright notice in the Description page of Project Settings.


#include "CraftingListWidget.h"

bool UCraftingListWidget::Toggle() {

	if (IsInViewport()) {
		RemoveFromParent();
		return false;
	}
	AddToViewport();
	return true;
}

