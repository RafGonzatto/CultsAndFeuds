#include "Net/SessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Steam/SteamAPIWrapper.h"

const FName USessionSubsystem::SESSION_NAME = TEXT("VazioSession");

USessionSubsystem::USessionSubsystem()
	: CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &USessionSubsystem::OnCreateSessionComplete))
	, FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &USessionSubsystem::OnFindSessionsComplete))
	// Delegates will be set up dynamically when needed
	, DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &USessionSubsystem::OnDestroySessionComplete))
	, StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &USessionSubsystem::OnStartSessionComplete))
{
}

void USessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		SessionInterface = OnlineSubsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("[SessionSubsystem] Online subsystem and session interface initialized successfully"));
		}
	}
}

void USessionSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void USessionSubsystem::HostArenaSession(const FString& MapName, const FString& Difficulty, int32 NumPublicConnections)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[SessionSubsystem] Cannot host session - SessionInterface is invalid"));
		OnCreateSessionCompleteEvent.Broadcast(SESSION_NAME, false);
		return;
	}

	// Store parameters
	CurrentMapName = MapName;
	CurrentDifficulty = Difficulty;

	// Create session settings
	FOnlineSessionSettings SessionSettings;
	CreateSessionSettings(SessionSettings, MapName, Difficulty, NumPublicConnections);

	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	// Create session
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), SESSION_NAME, SessionSettings))
	{
		UE_LOG(LogTemp, Error, TEXT("[SessionSubsystem] Failed to create session"));
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		OnCreateSessionCompleteEvent.Broadcast(SESSION_NAME, false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[SessionSubsystem] Creating session..."));
	}
}

void USessionSubsystem::FindFriendSessions()
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[SessionSubsystem] Cannot find sessions - SessionInterface is invalid"));
		OnFindSessionsCompleteEvent.Broadcast(false);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[SessionSubsystem] Finding friend sessions..."));

	// Create search settings
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = false;
	SessionSearch->MaxSearchResults = 20;
	SessionSearch->PingBucketSize = 50;
	
	// Search for sessions
	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef()))
	{
		UE_LOG(LogTemp, Error, TEXT("[SessionSubsystem] Failed to start session search"));
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		OnFindSessionsCompleteEvent.Broadcast(false);
	}
}

void USessionSubsystem::JoinSessionByIndex(int32 SessionIndex)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[SessionSubsystem] Cannot join session - SessionInterface is invalid"));
		OnJoinSessionCompleteEvent.Broadcast(SESSION_NAME, static_cast<int32>(EOnJoinSessionCompleteResult::UnknownError));
		return;
	}

	if (!SessionSearch.IsValid() || SessionIndex >= SessionSearch->SearchResults.Num() || SessionIndex < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[SessionSubsystem] Invalid session index: %d"), SessionIndex);
		OnJoinSessionCompleteEvent.Broadcast(SESSION_NAME, static_cast<int32>(EOnJoinSessionCompleteResult::SessionDoesNotExist));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[SessionSubsystem] Joining session at index %d..."), SessionIndex);

	// Set up join session delegate
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &USessionSubsystem::OnJoinSessionComplete));

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), SESSION_NAME, SessionSearch->SearchResults[SessionIndex]))
	{
		UE_LOG(LogTemp, Error, TEXT("[SessionSubsystem] Failed to join session"));
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		OnJoinSessionCompleteEvent.Broadcast(SESSION_NAME, static_cast<int32>(EOnJoinSessionCompleteResult::UnknownError));
	}
}

void USessionSubsystem::DestroySession()
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[SessionSubsystem] Cannot destroy session - SessionInterface is invalid"));
		OnDestroySessionCompleteEvent.Broadcast(SESSION_NAME, false);
		return;
	}

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if (!SessionInterface->DestroySession(SESSION_NAME))
	{
		UE_LOG(LogTemp, Error, TEXT("[SessionSubsystem] Failed to destroy session"));
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		OnDestroySessionCompleteEvent.Broadcast(SESSION_NAME, false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[SessionSubsystem] Destroying session..."));
	}
}

void USessionSubsystem::StartSession()
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[SessionSubsystem] Cannot start session - SessionInterface is invalid"));
		OnStartSessionCompleteEvent.Broadcast(SESSION_NAME, false);
		return;
	}

	StartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);

	if (!SessionInterface->StartSession(SESSION_NAME))
	{
		UE_LOG(LogTemp, Error, TEXT("[SessionSubsystem] Failed to start session"));
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
		OnStartSessionCompleteEvent.Broadcast(SESSION_NAME, false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[SessionSubsystem] Starting session..."));
	}
}

bool USessionSubsystem::IsSteamLoggedIn() const
{
	return SteamAPIWrapper::IsLoggedIn();
}

FString USessionSubsystem::GetPlayerName() const
{
	return SteamAPIWrapper::GetPlayerName();
}

FString USessionSubsystem::GetPlayerSteamName() const
{
	return SteamAPIWrapper::GetPlayerName();
}

int32 USessionSubsystem::GetSessionSearchResultsCount() const
{
	if (SessionSearch.IsValid())
	{
		return SessionSearch->SearchResults.Num();
	}
	return 0;
}

FString USessionSubsystem::GetSessionInfo(int32 SessionIndex) const
{
	if (!SessionSearch.IsValid() || SessionIndex >= SessionSearch->SearchResults.Num() || SessionIndex < 0)
	{
		return TEXT("Invalid session index");
	}

	const FOnlineSessionSearchResult& SearchResult = SessionSearch->SearchResults[SessionIndex];
	if (!SearchResult.IsValid())
	{
		return TEXT("Invalid session data");
	}

	FString SessionName;
	SearchResult.Session.SessionSettings.Get(FName("SessionName"), SessionName);
	
	FString MapName;
	SearchResult.Session.SessionSettings.Get(FName("MapName"), MapName);
	
	FString Difficulty;
	SearchResult.Session.SessionSettings.Get(FName("Difficulty"), Difficulty);

	int32 CurrentPlayers = SearchResult.Session.SessionSettings.NumPublicConnections - SearchResult.Session.NumOpenPublicConnections;
	int32 MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;

	return FString::Printf(TEXT("Session: %s | Map: %s | Difficulty: %s | Players: %d/%d"), 
		*SessionName, *MapName, *Difficulty, CurrentPlayers, MaxPlayers);
}

// Callbacks
void USessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("[SessionSubsystem] Create session complete: %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));
	
	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	OnCreateSessionCompleteEvent.Broadcast(SessionName, bWasSuccessful);
}

void USessionSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("[SessionSubsystem] Find sessions complete: %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));
	
	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SessionSubsystem] Found %d sessions"), SessionSearch->SearchResults.Num());
		
		// Log session details for debugging
		for (int32 i = 0; i < SessionSearch->SearchResults.Num(); i++)
		{
			const FOnlineSessionSearchResult& SearchResult = SessionSearch->SearchResults[i];
			if (SearchResult.IsValid())
			{
				FString SessionName;
				SearchResult.Session.SessionSettings.Get(FName("SessionName"), SessionName);
				
				FString MapName;
				SearchResult.Session.SessionSettings.Get(FName("MapName"), MapName);
				
				UE_LOG(LogTemp, Warning, TEXT("[SessionSubsystem] Session %d: %s - Map: %s - Players: %d/%d"), 
					i, *SessionName, *MapName, 
					SearchResult.Session.SessionSettings.NumPublicConnections - SearchResult.Session.NumOpenPublicConnections,
					SearchResult.Session.SessionSettings.NumPublicConnections);
			}
		}
	}
	
	OnFindSessionsCompleteEvent.Broadcast(bWasSuccessful);
}

void USessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogTemp, Warning, TEXT("[SessionSubsystem] Join session complete: %s"), Result == EOnJoinSessionCompleteResult::Success ? TEXT("Success") : TEXT("Failed"));
	
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		// Get the connection string and travel to the session
		FString ConnectionString;
		if (SessionInterface->GetResolvedConnectString(SessionName, ConnectionString))
		{
			UE_LOG(LogTemp, Warning, TEXT("[SessionSubsystem] Got connection string: %s"), *ConnectionString);
			
			// Travel to the session
			if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
			{
				PlayerController->ClientTravel(ConnectionString, ETravelType::TRAVEL_Absolute);
				UE_LOG(LogTemp, Warning, TEXT("[SessionSubsystem] Traveling to session..."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[SessionSubsystem] Failed to get connection string"));
		}
	}
	
	OnJoinSessionCompleteEvent.Broadcast(SessionName, static_cast<int32>(Result));
}

void USessionSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("[SessionSubsystem] Destroy session complete: %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));
	
	SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	OnDestroySessionCompleteEvent.Broadcast(SessionName, bWasSuccessful);
}

void USessionSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("[SessionSubsystem] Start session complete: %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));
	
	SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
	OnStartSessionCompleteEvent.Broadcast(SessionName, bWasSuccessful);
}

void USessionSubsystem::CreateSessionSettings(FOnlineSessionSettings& SessionSettings, const FString& MapName, const FString& Difficulty, int32 NumPublicConnections)
{
	SessionSettings.bIsLANMatch = false;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowInvites = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.NumPublicConnections = NumPublicConnections;
	SessionSettings.NumPrivateConnections = 0;

	// Custom settings
	SessionSettings.Set(FName("MapName"), MapName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(FName("Difficulty"), Difficulty, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
}

FString USessionSubsystem::GetCurrentMapName() const
{
	return CurrentMapName;
}

FString USessionSubsystem::GetCurrentDifficulty() const
{
	return CurrentDifficulty;
}
