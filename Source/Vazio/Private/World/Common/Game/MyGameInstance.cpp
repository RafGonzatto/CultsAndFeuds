#include "World/Common/Game/MyGameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Net/MatchFlowController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Steam/SteamAPIWrapper.h"

UMyGameInstance::UMyGameInstance()
{
}

void UMyGameInstance::Init()
{
	Super::Init();
	
	UE_LOG(LogTemp, Warning, TEXT("[MyGameInstance] Initializing..."));
	
	// Initialize Steam login
	InitializeSteamLogin();
}

void UMyGameInstance::Shutdown()
{
	// Clear timer
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(SteamLoginTimer);
	}
	
	// Shutdown Steam API
	SteamAPIWrapper::Shutdown();
	
	Super::Shutdown();
}

void UMyGameInstance::LoadComplete(const float LoadTime, const FString& MapName)
{
	Super::LoadComplete(LoadTime, MapName);
	
	UE_LOG(LogTemp, Warning, TEXT("[MyGameInstance] Load complete: %s (%.2fs)"), *MapName, LoadTime);
	
	// Hide loading screen when we reach Battle_Main
	if (MapName.Contains(TEXT("Battle_Main")))
	{
		UMatchFlowController* MatchFlow = GetSubsystem<UMatchFlowController>();
		if (MatchFlow)
		{
			// Small delay to ensure everything is loaded
			FTimerHandle DelayTimer;
			GetWorld()->GetTimerManager().SetTimer(DelayTimer, [MatchFlow]()
			{
				MatchFlow->HideLoadingScreen();
			}, 1.0f, false);
		}
	}
}

void UMyGameInstance::InitializeSteamLogin()
{
	// Initialize Steam API first
	if (!SteamAPIWrapper::Initialize())
	{
		UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] Failed to initialize Steam API"));
		return;
	}
	
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] No online subsystem found"));
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[MyGameInstance] Online subsystem: %s"), *OnlineSubsystem->GetSubsystemName().ToString());
	
	IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] Identity interface not available"));
		return;
	}
	
	// Check Steam API login status
	if (SteamAPIWrapper::IsLoggedIn())
	{
		UE_LOG(LogTemp, Warning, TEXT("[MyGameInstance] Logged in to Steam as: %s"), *SteamAPIWrapper::GetPlayerName());
	}
	
	// Check if already logged in to online subsystem
	ELoginStatus::Type LoginStatus = IdentityInterface->GetLoginStatus(0);
	if (LoginStatus == ELoginStatus::LoggedIn)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MyGameInstance] Already logged in to Steam"));
		return;
	}
	
	// Start automatic login process
	UE_LOG(LogTemp, Warning, TEXT("[MyGameInstance] Starting Steam auto-login..."));
	
	FOnlineAccountCredentials Credentials;
	Credentials.Id = FString();
	Credentials.Token = FString();
	Credentials.Type = FString("steam");
	
	IdentityInterface->Login(0, Credentials);
	
	// Set up timer to check login status
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(SteamLoginTimer, this, &UMyGameInstance::CheckSteamLoginStatus, 1.0f, true);
	}
}

void UMyGameInstance::CheckSteamLoginStatus()
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		return;
	}
	
	IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		return;
	}
	
	ELoginStatus::Type LoginStatus = IdentityInterface->GetLoginStatus(0);
	if (LoginStatus == ELoginStatus::LoggedIn)
	{
		FString PlayerName = IdentityInterface->GetPlayerNickname(0);
		UE_LOG(LogTemp, Warning, TEXT("[MyGameInstance] Steam login successful: %s"), *PlayerName);
		
		// Clear timer
		GetWorld()->GetTimerManager().ClearTimer(SteamLoginTimer);
	}
	else if (LoginStatus == ELoginStatus::NotLoggedIn)
	{
		// Could retry or show error after some attempts
		static int32 LoginAttempts = 0;
		LoginAttempts++;
		
		if (LoginAttempts > 10) // 10 seconds timeout
		{
			UE_LOG(LogTemp, Error, TEXT("[MyGameInstance] Steam login failed after %d attempts"), LoginAttempts);
			GetWorld()->GetTimerManager().ClearTimer(SteamLoginTimer);
		}
	}
}
