#include "Multiplayer/SteamMultiplayerSubsystem.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

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
    
    // TODO: Replace with actual Steam SDK initialization
    // For now, simulate Steam initialization with placeholder data
    InitializePlaceholderData();
    
    bIsSteamInitialized = true;
    OnSteamAuthComplete.Broadcast(true);
    
    // Automatically refresh friends list after successful initialization
    RefreshFriendsList();
    
    UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Steam initialized successfully (placeholder)"));
}

void USteamMultiplayerSubsystem::RefreshFriendsList()
{
    if (!bIsSteamInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Cannot refresh friends list - Steam not initialized"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Refreshing friends list..."));
    
    // TODO: Replace with actual Steam friends API calls
    UpdatePlaceholderFriends();
    
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
    
    // TODO: Implement actual Steam invite functionality
    // For now, simulate successful invite
    OnSessionInviteSent.Broadcast(FriendName, true);
    
    UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Invite sent to %s (placeholder)"), *FriendName);
}

void USteamMultiplayerSubsystem::JoinFriendSession(const FString& FriendSteamID)
{
    if (!bIsSteamInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Cannot join friend session - Steam not initialized"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Attempting to join friend's session: %s"), *FriendSteamID);
    
    // TODO: Implement actual Steam session joining
    // For now, just log the attempt
    UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Joined friend's session (placeholder)"));
}

void USteamMultiplayerSubsystem::InitializePlaceholderData()
{
    // Simulate getting player's Steam data
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
    
    UE_LOG(LogTemp, Warning, TEXT("[SteamSubsystem] Initialized placeholder data - Player: %s (%s)"), *PlayerSteamName, *PlayerSteamID);
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
