// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Interfaces/OnlineSessionInterface.h"
#include "UObject/Object.h"
#include "MyGameInstance.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FFoundServerInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString ServerName;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentPlayers;

	UPROPERTY(BlueprintReadOnly)
	FString HostName;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnServersFound, const TArray<FFoundServerInfo>&, FoundServers);

UCLASS()
class SURVIVALFPS_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

private:
	bool bHasSeed = false;
	int32 Seed;

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

	bool bIsHosting = false;

public:
	UPROPERTY(BlueprintAssignable)
	FOnServersFound OnServersFound;

protected:
	void OnCreateSessionComplete(FName ServerName, bool Success);

	void OnDestroySessionComplete(FName ServerName, bool Success);
	
	void OnFindSessionsComplete(bool Success);

	void OnJoinSessionComplete(FName ServerName, EOnJoinSessionCompleteResult::Type JoinResult);
	
	
public:

	bool IsSeedSet()
	{
		return bHasSeed;
	};

	int32 GetSeed()
	{
		return Seed;
	}
	
	void SetSeed(int32 NewSeed)
	{
		bHasSeed = true;
		Seed = NewSeed;
	}

	
	
	void ResetSeed()
	{
		bHasSeed = false;
	}

	virtual void Init() override;
	virtual void Shutdown() override;

	void Host(FString ServerName);

	UFUNCTION(BlueprintCallable)
	void JoinSession(int32 Index);

	UFUNCTION(BlueprintCallable)
	void FindSessions();
	void EndSession();

};
