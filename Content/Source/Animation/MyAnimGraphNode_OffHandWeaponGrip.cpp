// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAnimGraphNode_OffHandWeaponGrip.h"

#define LOCTEXT_NAMESPACE "MyEditor_Node"

FText UMyAnimGraphNode_OffHandWeaponGrip::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return GetControllerDescription();
}

FLinearColor UMyAnimGraphNode_OffHandWeaponGrip::GetNodeTitleColor() const
{
	return FLinearColor::Black;
}

FText UMyAnimGraphNode_OffHandWeaponGrip::GetTooltipText() const
{
	return LOCTEXT("AnimGraphNode_OffHandWeaponGrip_Tooltip", "Procedurally places the off-hand on a weapon.");
}

FString UMyAnimGraphNode_OffHandWeaponGrip::GetNodeCategory() const
{
	return TEXT("MyAnimation");
}

FText UMyAnimGraphNode_OffHandWeaponGrip::GetControllerDescription() const
{
	return LOCTEXT("OffHandWeaponGrip", "Off-Hand Weapon Grip");
}

#undef LOCTEXT_NAMESPACE
