#include "Net/SessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

const FName USessionSubsystem::SESSION_NAME = TEXT("VazioSession");

USessionSubsystem::USessionSubsystem()
	: CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &USessionSubsystem::OnCreateSessionComplete))
	, FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &USessionSubsystem::OnFindSessionsComplete))
	, JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &USessionSubsystem::OnJoinSessionComplete))
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

	// Simple implementation - just broadcast success for now
	UE_LOG(LogTemp, Warning, TEXT("[SessionSubsystem] Finding sessions..."));
	OnFindSessionsCompleteEvent.Broadcast(true);
}

void USessionSubsystem::JoinSessionByIndex(int32 SessionIndex)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[SessionSubsystem] Cannot join session - SessionInterface is invalid"));
		OnJoinSessionCompleteEvent.Broadcast(SESSION_NAME, 1); // Error
		return;
	}

	// Simple implementation - just broadcast success for now
	UE_LOG(LogTemp, Warning, TEXT("[SessionSubsystem] Joining session at index %d..."), SessionIndex);
	OnJoinSessionCompleteEvent.Broadcast(SESSION_NAME, 0); // Success
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
	// Simple implementation - return true for now
	return true;
}

FString USessionSubsystem::GetPlayerName() const
{
	return TEXT("Player");
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
	
	OnFindSessionsCompleteEvent.Broadcast(bWasSuccessful);
}

void USessionSubsystem::OnJoinSessionComplete(FName SessionName, int32 Result)
{
	UE_LOG(LogTemp, Warning, TEXT("[SessionSubsystem] Join session complete: %s"), Result == 0 ? TEXT("Success") : TEXT("Failed"));
	
	OnJoinSessionCompleteEvent.Broadcast(SessionName, Result);
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
