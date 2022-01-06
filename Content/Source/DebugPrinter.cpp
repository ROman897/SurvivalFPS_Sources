// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugPrinter.h"

FString UDebugPrinter::GetPIENetMode(UWorld* WorldContext)
{
	check(WorldContext);
	if (WorldContext->WorldType != EWorldType::PIE)
	{
		return "not PIE";
	}
	switch (WorldContext->GetNetMode())
	{
		case NM_Client:
			return FString::Printf(TEXT("Client %d: "), GPlayInEditorID);
		case NM_DedicatedServer:
		case NM_ListenServer:
			return FString::Printf(TEXT("Server: "));
		default:
			return FString("unknown: ");
	}
}
