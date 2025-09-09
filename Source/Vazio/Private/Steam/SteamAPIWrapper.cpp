#include "Steam/SteamAPIWrapper.h"
#include "Engine/Engine.h"
#include "Multiplayer/SteamMultiplayerSubsystem.h" // For FSteamFriend struct
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

bool SteamAPIWrapper::bInitialized = false;
static void* SteamDLLHandle = nullptr;

bool SteamAPIWrapper::Initialize()
{
    // Always return true for now to prevent crashes - we'll implement Steam in packaged builds only
    UE_LOG(LogTemp, Warning, TEXT("[SteamAPI] Steam API disabled in current build - running in safe mode"));
    bInitialized = true;
    return true;

#if 0 // Temporarily disabled to prevent crashes
#if STEAM_SDK_AVAILABLE
    if (!bInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SteamAPI] Attempting to initialize Steam API safely..."));
        
        // First check if Steam DLL exists and try to load it dynamically
        FString SteamDLLPath = FPaths::Combine(FPaths::ProjectDir(), TEXT("ThirdParty/Steamworks/lib/steam_api64.dll"));
        
        if (!FPaths::FileExists(SteamDLLPath))
        {
            UE_LOG(LogTemp, Warning, TEXT("[SteamAPI] Steam DLL not found at: %s"), *SteamDLLPath);
            UE_LOG(LogTemp, Warning, TEXT("[SteamAPI] Running in DEVELOPMENT MODE - Steam features disabled"));
            bInitialized = true;
            return true;
        }
        
        // Try to load the DLL dynamically first
        SteamDLLHandle = FPlatformProcess::GetDllHandle(*SteamDLLPath);
        if (!SteamDLLHandle)
        {
            UE_LOG(LogTemp, Warning, TEXT("[SteamAPI] Could not load Steam DLL dynamically"));
            UE_LOG(LogTemp, Warning, TEXT("[SteamAPI] Running in DEVELOPMENT MODE - Steam features disabled"));
            bInitialized = true;
            return true;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("[SteamAPI] Steam DLL loaded successfully, attempting API init..."));
        
        try
        {
            // Now try to initialize Steam API
            if (SteamAPI_Init())
            {
                bInitialized = true;
                UE_LOG(LogTemp, Warning, TEXT("[SteamAPI] ✅ Steam API initialized successfully - REAL STEAM MODE"));
                return true;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("[SteamAPI] ⚠️ Steam API init failed, trying restart approach..."));
            }
        }
        catch (...)
        {
            UE_LOG(LogTemp, Error, TEXT("[SteamAPI] ❌ Exception during Steam API init, continuing in dev mode..."));
        }
        
        // Try restart approach as fallback
        if (SteamAPI_RestartAppIfNecessary(480)) // Replace 480 with your actual App ID
        {
            // If this returns true, it means the game was not launched through Steam
            UE_LOG(LogTemp, Warning, TEXT("Game was not launched through Steam"));
            UE_LOG(LogTemp, Warning, TEXT("[SteamAPI] Running in DEVELOPMENT MODE - will try to access Steam if available"));
            bInitialized = true; // Allow development mode
            return true;
        }
        
        if (!SteamAPI_Init())
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to initialize Steam API - Running in DEVELOPMENT MODE"));
            UE_LOG(LogTemp, Warning, TEXT("[SteamAPI] Will attempt to use Steam features if Steam is running"));
            bInitialized = true; // Allow development mode
            return true;
        }
        
        bInitialized = true;
        UE_LOG(LogTemp, Warning, TEXT("Steam API initialized successfully - REAL STEAM MODE"));
        return true;
    }
    return true;
#else
    UE_LOG(LogTemp, Warning, TEXT("Steam SDK not available - using placeholder mode"));
    bInitialized = true;
    return true;
#endif
#endif // End of disabled code block
}

void SteamAPIWrapper::Shutdown()
{
#if STEAM_SDK_AVAILABLE
    if (bInitialized)
    {
        try
        {
            SteamAPI_Shutdown();
        }
        catch (...)
        {
            UE_LOG(LogTemp, Warning, TEXT("[SteamAPI] Exception during shutdown - continuing"));
        }
        
        if (SteamDLLHandle)
        {
            FPlatformProcess::FreeDllHandle(SteamDLLHandle);
            SteamDLLHandle = nullptr;
            UE_LOG(LogTemp, Warning, TEXT("[SteamAPI] Steam DLL unloaded"));
        }
        
        bInitialized = false;
        UE_LOG(LogTemp, Warning, TEXT("[SteamAPI] Steam API shutdown completed"));
    }
#endif
}

bool SteamAPIWrapper::IsInitialized()
{
    return bInitialized;
}

bool SteamAPIWrapper::IsLoggedIn()
{
    // Disabled to prevent crashes - always return false
    return false;
}

FString SteamAPIWrapper::GetPlayerName()
{
    // Disabled to prevent crashes - return placeholder
    return TEXT("Development Player");
}

uint64 SteamAPIWrapper::GetSteamID()
{
    // Disabled to prevent crashes - return placeholder ID
    return 12345678901234567;
}

// Try to get real Steam friends even in development mode
bool SteamAPIWrapper::TryGetRealSteamFriends(TArray<struct FSteamFriend>& OutFriends)
{
    // Temporarily disabled to prevent crashes - return false to use mock data
    UE_LOG(LogTemp, Warning, TEXT("[SteamAPI] TryGetRealSteamFriends disabled - using mock data instead"));
    return false;

#if 0 // Disabled Steam friends functionality
#if STEAM_SDK_AVAILABLE
    // Try to initialize Steam if not already done and Steam is running
    if (!IsLoggedIn() && SteamAPI_IsSteamRunning())
    {
        UE_LOG(LogTemp, Warning, TEXT("[SteamAPI] Steam is running, attempting lightweight connection for friends..."));
        if (SteamAPI_Init())
        {
            bInitialized = true;
            UE_LOG(LogTemp, Warning, TEXT("[SteamAPI] Successfully connected to Steam for friends access!"));
        }
    }
    
    if (bInitialized && SteamFriends())
    {
        OutFriends.Empty();
        
        int32 FriendCount = SteamFriends()->GetFriendCount(k_EFriendFlagImmediate);
        UE_LOG(LogTemp, Warning, TEXT("[SteamAPI] Found %d real Steam friends"), FriendCount);
        
        for (int32 i = 0; i < FriendCount; i++)
        {
            CSteamID FriendSteamID = SteamFriends()->GetFriendByIndex(i, k_EFriendFlagImmediate);
            
            // Get friend info
            FString FriendName = UTF8_TO_TCHAR(SteamFriends()->GetFriendPersonaName(FriendSteamID));
            FString FriendID = FString::Printf(TEXT("%llu"), FriendSteamID.ConvertToUint64());
            
            EPersonaState PersonaState = SteamFriends()->GetFriendPersonaState(FriendSteamID);
            bool bIsOnline = (PersonaState > k_EPersonaStateOffline);
            
            // Check if friend is in a game
            FriendGameInfo_t GameInfo;
            bool bIsInGame = SteamFriends()->GetFriendGamePlayed(FriendSteamID, &GameInfo) && GameInfo.m_gameID.IsValid();
            
            // Add friend to list
            FSteamFriend Friend(FriendName, FriendID, bIsOnline, bIsInGame);
            OutFriends.Add(Friend);
            
            UE_LOG(LogTemp, Log, TEXT("[SteamAPI] REAL Friend: %s (%s) - Online: %s, InGame: %s"),
                   *FriendName, *FriendID, bIsOnline ? TEXT("Yes") : TEXT("No"), bIsInGame ? TEXT("Yes") : TEXT("No"));
        }
        
        return true;
    }
#endif
    return false;
#endif // End of disabled Steam friends code
}
