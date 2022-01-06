// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

void UMyGameInstance::OnCreateSessionComplete(FName ServerName, bool Success)
{
	UE_LOG(LogTemp, Warning, TEXT("OnCreateSessionComplete"));
}

void UMyGameInstance::OnDestroySessionComplete(FName ServerName, bool Success)
{
}

void UMyGameInstance::OnFindSessionsComplete(bool Success)
{
	UE_LOG(LogTemp, Warning, TEXT("OnFindSessionsComplete"));
	if (!Success || !SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnFindSessionsComplete failed"));
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("OnFindSessionsComplete succeed"));

	TArray<FFoundServerInfo> FoundServers;
	for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
	{
		FFoundServerInfo FoundServerInfo;
		
		FString ServerName;
		if (SearchResult.Session.SessionSettings.Get("ServerName", ServerName))
		{
			FoundServerInfo.ServerName = ServerName;
		}
		FoundServerInfo.CurrentPlayers = SearchResult.Session.SessionSettings.NumPublicConnections -  SearchResult.Session.NumOpenPublicConnections;
		FoundServerInfo.HostName = SearchResult.Session.OwningUserName;
		FoundServers.Add(FoundServerInfo);
	}

	OnServersFound.Broadcast(FoundServers);
}

void UMyGameInstance::OnJoinSessionComplete(FName ServerName, EOnJoinSessionCompleteResult::Type JoinResult)
{
	FString URL;
	if (!SessionInterface->GetResolvedConnectString(ServerName, URL))
	{
		return;
	}

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	PlayerController->ClientTravel(URL, ETravelType::TRAVEL_Absolute);
}

void UMyGameInstance::Init()
{
	Super::Init();
	
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMyGameInstance::OnCreateSessionComplete);
		SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UMyGameInstance::OnDestroySessionComplete);
		SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMyGameInstance::OnFindSessionsComplete);
		SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMyGameInstance::OnJoinSessionComplete);
	}
		
}

void UMyGameInstance::Shutdown()
{
	if (bIsHosting)
	{
		EndSession();
	}
	Super::Shutdown();
}

void UMyGameInstance::Host(FString ServerName)
{
	UE_LOG(LogTemp, Warning, TEXT("try host online session"));
	if (!SessionInterface.IsValid())
	{
	UE_LOG(LogTemp, Warning, TEXT("host online session failed"));
		return;
	}

	FOnlineSessionSettings OnlineSessionSettings;
	OnlineSessionSettings.bIsLANMatch = false;
	OnlineSessionSettings.NumPublicConnections = 10;
	OnlineSessionSettings.bShouldAdvertise = true;
	OnlineSessionSettings.bUsesPresence = true;
	OnlineSessionSettings.Set("ServerName", ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	if (SessionInterface->CreateSession(0, "TestSession2", OnlineSessionSettings))
	{
		bIsHosting = true;
		UE_LOG(LogTemp, Warning, TEXT("created online session"));
	}
}

void UMyGameInstance::JoinSession(int32 Index)
{
	FName SessionName("TestSession2");
	//FName ServerName("ServerName");
	//SessionSearch->SearchResults[Index].Session.SessionSettings.Get(ServerName, SessionName);
	SessionInterface->JoinSession(0, SessionName, SessionSearch->SearchResults[Index]);
}

void UMyGameInstance::FindSessions()
{
	UE_LOG(LogTemp, Warning, TEXT("FindSessions called"));
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	
    if (SessionSearch.IsValid())
    {
		UE_LOG(LogTemp, Warning, TEXT("FindSessions SessionSearch.IsValid"));
       // Set properties
       SessionSearch->MaxSearchResults = 100;
       //SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true,
       //                          EOnlineComparisonOp::Equals);
       SessionInterface->FindSessions(0,
           SessionSearch.ToSharedRef());
     }
 }

void UMyGameInstance::EndSession()
{
	bIsHosting = false;
	check(SessionInterface != nullptr)
	{
		SessionInterface->EndSession("TestSession2");
	}
}
