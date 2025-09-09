#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "FindSessionsCallbackProxy.h"
#include "SessionSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSessionCreateComplete, FName, SessionName, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSessionFindComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSessionJoinComplete, FName, SessionName, int32, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSessionDestroyComplete, FName, SessionName, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSessionStartComplete, FName, SessionName, bool, bWasSuccessful);

UCLASS()
class VAZIO_API USessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USessionSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Session Management
	UFUNCTION(BlueprintCallable)
	void HostArenaSession(const FString& MapName, const FString& Difficulty, int32 NumPublicConnections = 4);
	
	UFUNCTION(BlueprintCallable)
	void FindFriendSessions();
	
	UFUNCTION(BlueprintCallable)
	void JoinSessionByIndex(int32 SessionIndex);
	
	UFUNCTION(BlueprintCallable)
	void DestroySession();
	
	UFUNCTION(BlueprintCallable)
	void StartSession();

	// Steam Integration
	UFUNCTION(BlueprintCallable)
	bool IsSteamLoggedIn() const;
	
	UFUNCTION(BlueprintCallable)
	FString GetPlayerName() const;
	
	UFUNCTION(BlueprintCallable)
	FString GetPlayerSteamName() const;

	// Get current session parameters
	UFUNCTION(BlueprintCallable)
	FString GetCurrentMapName() const;
	
	UFUNCTION(BlueprintCallable)
	FString GetCurrentDifficulty() const;

	// Get session search results
	UFUNCTION(BlueprintCallable)
	int32 GetSessionSearchResultsCount() const;

	UFUNCTION(BlueprintCallable)
	FString GetSessionInfo(int32 SessionIndex) const;

	// Delegates
	UPROPERTY(BlueprintAssignable)
	FSessionCreateComplete OnCreateSessionCompleteEvent;
	
	UPROPERTY(BlueprintAssignable)
	FSessionFindComplete OnFindSessionsCompleteEvent;
	
	UPROPERTY(BlueprintAssignable)
	FSessionJoinComplete OnJoinSessionCompleteEvent;
	
	UPROPERTY(BlueprintAssignable)
	FSessionDestroyComplete OnDestroySessionCompleteEvent;
	
	UPROPERTY(BlueprintAssignable)
	FSessionStartComplete OnStartSessionCompleteEvent;

private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;
	
	// Session callbacks
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	
	// Delegate handles
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	// Session callbacks implementation
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result); // changed from int32
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

	// Session settings helper
	void CreateSessionSettings(FOnlineSessionSettings& SessionSettings, const FString& MapName, const FString& Difficulty, int32 NumPublicConnections);
	
	// Current session info
	FString CurrentMapName;
	FString CurrentDifficulty;
	static const FName SESSION_NAME;
};
