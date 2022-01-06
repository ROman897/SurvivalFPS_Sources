// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPropsReplacer.h"

#include "InGameResource.h"
#include "InstancedEnvironmentResourcePropComponent.h"

UPlayerPropsReplacer::UPlayerPropsReplacer()
{
	OnComponentBeginOverlap.AddDynamic(this, &UPlayerPropsReplacer::OnPropReplacementTriggerBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UPlayerPropsReplacer::OnPropActorReplacementTriggerEndOverlap);
}

void UPlayerPropsReplacer::OnPropReplacementTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UInstancedEnvironmentResourcePropComponent* InstancedEnvironmentResourcePropComponent = Cast<UInstancedEnvironmentResourcePropComponent>(OtherComp);
	if (InstancedEnvironmentResourcePropComponent == nullptr)
	{
		return;
	}
	InstancedEnvironmentResourcePropComponent->ReplacePropByActor(OtherBodyIndex);
}

void UPlayerPropsReplacer::OnPropActorReplacementTriggerEndOverlap(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AInGameResource* InGameResource = Cast<AInGameResource>(OtherActor);

	if (IsValid(InGameResource))
	{
		InGameResource->GetReclaimed();
	}
}
