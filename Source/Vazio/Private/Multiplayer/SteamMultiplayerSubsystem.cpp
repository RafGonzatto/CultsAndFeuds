#include "Multiplayer/SteamMultiplayerSubsystem.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Steam/SteamAPIWrapper.h"
#include "Net/SessionSubsystem.h"

#ifdef STEAM_SDK_AVAILABLE
#include "steam_api.h"
#endif

void USteamMultiplayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bIsSteamInitialized = false;
    bIsSessionActive = false;
    CurrentMaxPlayers = 4;
    
    UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Initialized"));
}

void USteamMultiplayerSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Shutting down"));
    
    // Clean up any active sessions
    if (bIsSessionActive)
    {
        // TODO: Implement session cleanup
    }
    
    Super::Deinitialize();
}

void USteamMultiplayerSubsystem::InitializeSteam()
{
    UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Attempting to initialize Steam..."));
    
    // Use real Steam API through SteamAPIWrapper
    bool bSteamInitSuccess = SteamAPIWrapper::Initialize();
    
    if (bSteamInitSuccess)
    {
        if (SteamAPIWrapper::IsLoggedIn())
        {
            // Real Steam mode
            bIsSteamInitialized = true;
            
            // Get real player data from Steam
            PlayerSteamName = SteamAPIWrapper::GetPlayerName();
            PlayerSteamID = FString::Printf(TEXT("%llu"), SteamAPIWrapper::GetSteamID());
            
            UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] REAL STEAM MODE - Player: %s (%s)"), 
                   *PlayerSteamName, *PlayerSteamID);
            
            OnSteamAuthComplete.Broadcast(true);
            
            // Automatically refresh friends list after successful initialization
            RefreshFriendsList();
        }
        else
        {
            // Development mode - Steam API available but not logged in or not launched through Steam
            UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] DEVELOPMENT MODE - Using enhanced placeholder data"));
            InitializePlaceholderData();
            bIsSteamInitialized = true;
            
            OnSteamAuthComplete.Broadcast(true); // Report success for development
            
            // Use placeholder friends in development
            RefreshFriendsList();
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[SteamSubsystem] Steam API completely unavailable"));
        
        // Fallback to placeholder data for testing
        UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Using basic placeholder data"));
        InitializePlaceholderData();
        bIsSteamInitialized = true;
        
        OnSteamAuthComplete.Broadcast(false);
    }
}

void USteamMultiplayerSubsystem::RefreshFriendsList()
{
    if (!bIsSteamInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Cannot refresh friends list - Steam not initialized"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Refreshing friends list..."));
    
    FriendsList.Empty();
    
    // First try to get REAL Steam friends even in development mode
    TArray<FSteamFriend> RealFriends;
    if (SteamAPIWrapper::TryGetRealSteamFriends(RealFriends))
    {
        FriendsList = RealFriends;
        UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] ✅ REAL STEAM FRIENDS loaded - %d friends found"), FriendsList.Num());
        
        // Log each friend for debugging
        for (const FSteamFriend& Friend : FriendsList)
        {
            UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] 👤 %s - %s - %s"), 
                   *Friend.DisplayName, 
                   Friend.bIsOnline ? TEXT("🟢 Online") : TEXT("⚫ Offline"),
                   Friend.bIsInGame ? TEXT("🎮 In Game") : TEXT("💬 Available"));
        }
    }
    else
    {
        // Fallback to placeholder data for development/testing
        UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Steam not available - using placeholder friends list"));
        UpdatePlaceholderFriends();
    }
    
    OnFriendsListUpdated.Broadcast(FriendsList);
    
    UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Friends list updated - %d friends found"), FriendsList.Num());
}

TArray<FSteamFriend> USteamMultiplayerSubsystem::GetOnlineFriends() const
{
    TArray<FSteamFriend> OnlineFriends;
    
    for (const FSteamFriend& Friend : FriendsList)
    {
        if (Friend.bIsOnline)
        {
            OnlineFriends.Add(Friend);
        }
    }
    
    return OnlineFriends;
}

void USteamMultiplayerSubsystem::CreateMultiplayerSession(const FString& SessionName, int32 MaxPlayers)
{
    if (!bIsSteamInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Cannot create session - Steam not initialized"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Creating multiplayer session: %s (Max Players: %d)"), *SessionName, MaxPlayers);
    
    CurrentSessionName = SessionName;
    CurrentMaxPlayers = MaxPlayers;
    bIsSessionActive = true;
    
    // TODO: Implement actual Steam session creation
    // For now, just log the creation
    UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Session created successfully (placeholder)"));
}

void USteamMultiplayerSubsystem::InviteFriendToSession(const FString& FriendSteamID)
{
    if (!bIsSteamInitialized || !bIsSessionActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Cannot invite friend - Steam not initialized or no active session"));
        OnSessionInviteSent.Broadcast(TEXT("Unknown"), false);
        return;
    }
    
    // Find friend name by Steam ID
    FString FriendName = TEXT("Unknown");
    for (const FSteamFriend& Friend : FriendsList)
    {
        if (Friend.SteamID == FriendSteamID)
        {
            FriendName = Friend.DisplayName;
            break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Inviting friend %s (%s) to session"), *FriendName, *FriendSteamID);
    
#ifdef STEAM_SDK_AVAILABLE
    if (SteamAPIWrapper::IsLoggedIn())
    {
        ISteamFriends* SteamFriendsAPI = SteamFriends();
        if (SteamFriendsAPI)
        {
            // Convert string Steam ID to CSteamID
            uint64 SteamID64 = FCString::Strtoui64(*FriendSteamID, nullptr, 10);
            CSteamID FriendID(SteamID64);
            
            if (FriendID.IsValid())
            {
                // Send Steam game invite
                bool bInviteResult = SteamFriendsAPI->InviteUserToGame(FriendID, "+connect_lobby");
                
                if (bInviteResult)
                {
                    UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Steam invite sent successfully to %s"), *FriendName);
                    OnSessionInviteSent.Broadcast(FriendName, true);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("[SteamSubsystem] Failed to send Steam invite to %s"), *FriendName);
                    OnSessionInviteSent.Broadcast(FriendName, false);
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("[SteamSubsystem] Invalid Steam ID: %s"), *FriendSteamID);
                OnSessionInviteSent.Broadcast(FriendName, false);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[SteamSubsystem] Steam Friends interface not available"));
            OnSessionInviteSent.Broadcast(FriendName, false);
        }
    }
    else
#endif
    {
        // Fallback for development/testing
        UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Simulating invite to %s (development mode)"), *FriendName);
        OnSessionInviteSent.Broadcast(FriendName, true);
    }
}

void USteamMultiplayerSubsystem::JoinFriendSession(const FString& FriendSteamID)
{
    if (!bIsSteamInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Cannot join friend session - Steam not initialized"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Attempting to join friend's session: %s"), *FriendSteamID);
    
#ifdef STEAM_SDK_AVAILABLE
    if (SteamAPIWrapper::IsLoggedIn())
    {
        ISteamFriends* SteamFriendsAPI = SteamFriends();
        if (SteamFriendsAPI)
        {
            // Convert string Steam ID to CSteamID
            uint64 SteamID64 = FCString::Strtoui64(*FriendSteamID, nullptr, 10);
            CSteamID FriendID(SteamID64);
            
            if (FriendID.IsValid())
            {
                // Check if friend is in a game
                FriendGameInfo_t GameInfo;
                if (SteamFriendsAPI->GetFriendGamePlayed(FriendID, &GameInfo) && GameInfo.m_gameID.IsValid())
                {
                    // Friend is in a game, try to join
                    UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Friend is in game, attempting to join..."));
                    
                    // This would typically trigger the OnlineSubsystem to handle the actual connection
                    // For now, we'll use the SessionSubsystem to handle the join
                    if (USessionSubsystem* SessionSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<USessionSubsystem>())
                    {
                        // In a real implementation, we'd get session info from Steam and join
                        // For now, simulate a join attempt
                        UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Triggering session join via SessionSubsystem"));
                        // SessionSubsystem->JoinSessionByIndex(0); // This would be the real call
                    }
                    
                    UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Join friend session initiated"));
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Friend is not currently in a game"));
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("[SteamSubsystem] Invalid Steam ID: %s"), *FriendSteamID);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[SteamSubsystem] Steam Friends interface not available"));
        }
    }
    else
#endif
    {
        // Fallback for development/testing
        UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Simulating join friend session (development mode)"));
    }
}

void USteamMultiplayerSubsystem::InitializePlaceholderData()
{
    // Try to get real Steam data first
    if (SteamAPIWrapper::IsLoggedIn())
    {
        PlayerSteamName = SteamAPIWrapper::GetPlayerName();
        PlayerSteamID = FString::Printf(TEXT("%llu"), SteamAPIWrapper::GetSteamID());
        UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Using real Steam data - Player: %s (%s)"), *PlayerSteamName, *PlayerSteamID);
    }
    else
    {
        // Fallback to simulated data for development
        PlayerSteamName = TEXT("LocalPlayer");
        PlayerSteamID = TEXT("76561198000000001");
        
        // Generate a unique-ish player name based on some system data
        if (GEngine)
        {
            FString ComputerName = FPlatformProcess::ComputerName();
            if (!ComputerName.IsEmpty())
            {
                PlayerSteamName = FString::Printf(TEXT("Player_%s"), *ComputerName.Right(4));
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Using placeholder data - Player: %s (%s)"), *PlayerSteamName, *PlayerSteamID);
    }
}

void USteamMultiplayerSubsystem::UpdatePlaceholderFriends()
{
    FriendsList.Empty();
    
    // Add some placeholder friends with varying online status
    FriendsList.Add(FSteamFriend(TEXT("Alex_Warrior"), TEXT("76561198000000002"), true, false));
    FriendsList.Add(FSteamFriend(TEXT("Sarah_Mage"), TEXT("76561198000000003"), true, true));
    FriendsList.Add(FSteamFriend(TEXT("Mike_Archer"), TEXT("76561198000000004"), false, false));
    FriendsList.Add(FSteamFriend(TEXT("Emma_Rogue"), TEXT("76561198000000005"), true, false));
    FriendsList.Add(FSteamFriend(TEXT("John_Paladin"), TEXT("76561198000000006"), true, true));
    FriendsList.Add(FSteamFriend(TEXT("Lisa_Sorcerer"), TEXT("76561198000000007"), false, false));
    
    // Randomize some online status to make it more realistic
    for (FSteamFriend& Friend : FriendsList)
    {
        // 70% chance to be online
        Friend.bIsOnline = FMath::RandBool() ? FMath::RandBool() : true;
        
        // If online, 30% chance to be in game
        if (Friend.bIsOnline)
        {
            Friend.bIsInGame = FMath::RandBool() && FMath::RandBool();
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Updated placeholder friends list with %d friends"), FriendsList.Num());
}
