#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SteamMultiplayerSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FSteamFriend
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Steam")
    FString DisplayName;

    UPROPERTY(BlueprintReadOnly, Category = "Steam")
    FString SteamID;

    UPROPERTY(BlueprintReadOnly, Category = "Steam")
    bool bIsOnline;

    UPROPERTY(BlueprintReadOnly, Category = "Steam")
    bool bIsInGame;

    FSteamFriend()
    {
        DisplayName = TEXT("");
        SteamID = TEXT("");
        bIsOnline = false;
        bIsInGame = false;
    }

    FSteamFriend(const FString& InDisplayName, const FString& InSteamID, bool bInIsOnline = true, bool bInIsInGame = false)
    {
        DisplayName = InDisplayName;
        SteamID = InSteamID;
        bIsOnline = bInIsOnline;
        bIsInGame = bInIsInGame;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSteamAuthComplete, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFriendsListUpdated, const TArray<FSteamFriend>&, Friends);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSessionInviteSent, const FString&, FriendName, bool, bSuccess);

UCLASS()
class VAZIO_API USteamMultiplayerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Steam Authentication
    UFUNCTION(BlueprintCallable, Category = "Steam")
    void InitializeSteam();

    UFUNCTION(BlueprintCallable, Category = "Steam")
    bool IsSteamInitialized() const { return bIsSteamInitialized; }

    UFUNCTION(BlueprintCallable, Category = "Steam")
    FString GetPlayerSteamName() const { return PlayerSteamName; }

    UFUNCTION(BlueprintCallable, Category = "Steam")
    FString GetPlayerSteamID() const { return PlayerSteamID; }

    // Friends Management
    UFUNCTION(BlueprintCallable, Category = "Steam")
    void RefreshFriendsList();

    UFUNCTION(BlueprintCallable, Category = "Steam")
    TArray<FSteamFriend> GetFriendsList() const { return FriendsList; }

    UFUNCTION(BlueprintCallable, Category = "Steam")
    TArray<FSteamFriend> GetOnlineFriends() const;

    // Multiplayer Session
    UFUNCTION(BlueprintCallable, Category = "Steam")
    void CreateMultiplayerSession(const FString& SessionName, int32 MaxPlayers = 4);

    UFUNCTION(BlueprintCallable, Category = "Steam")
    void InviteFriendToSession(const FString& FriendSteamID);

    UFUNCTION(BlueprintCallable, Category = "Steam")
    void JoinFriendSession(const FString& FriendSteamID);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Steam")
    FOnSteamAuthComplete OnSteamAuthComplete;

    UPROPERTY(BlueprintAssignable, Category = "Steam")
    FOnFriendsListUpdated OnFriendsListUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Steam")
    FOnSessionInviteSent OnSessionInviteSent;

protected:
    // Steam state
    UPROPERTY()
    bool bIsSteamInitialized;

    UPROPERTY()
    FString PlayerSteamName;

    UPROPERTY()
    FString PlayerSteamID;

    UPROPERTY()
    TArray<FSteamFriend> FriendsList;

    // Current session info
    UPROPERTY()
    FString CurrentSessionName;

    UPROPERTY()
    int32 CurrentMaxPlayers;

    UPROPERTY()
    bool bIsSessionActive;

private:
    // Initialize player data (real Steam data when available, placeholder for development)
    void InitializePlaceholderData();

    // Update friends list with placeholder data (fallback for development/testing)
    void UpdatePlaceholderFriends();
};
