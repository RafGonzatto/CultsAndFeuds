#include "Steam/SteamAPIWrapper.h"
#include "Engine/Engine.h"

bool SteamAPIWrapper::bInitialized = false;

bool SteamAPIWrapper::Initialize()
{
#if STEAM_SDK_AVAILABLE
    if (!bInitialized)
    {
        if (SteamAPI_RestartAppIfNecessary(480)) // Replace 480 with your actual App ID
        {
            // If this returns true, it means the game was not launched through Steam
            UE_LOG(LogTemp, Warning, TEXT("Game was not launched through Steam, restarting..."));
            return false;
        }
        
        if (!SteamAPI_Init())
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to initialize Steam API"));
            return false;
        }
        
        bInitialized = true;
        UE_LOG(LogTemp, Warning, TEXT("Steam API initialized successfully"));
        return true;
    }
    return true;
#else
    UE_LOG(LogTemp, Warning, TEXT("Steam SDK not available - using placeholder mode"));
    bInitialized = true;
    return true;
#endif
}

void SteamAPIWrapper::Shutdown()
{
#if STEAM_SDK_AVAILABLE
    if (bInitialized)
    {
        SteamAPI_Shutdown();
        bInitialized = false;
        UE_LOG(LogTemp, Warning, TEXT("Steam API shutdown"));
    }
#endif
}

bool SteamAPIWrapper::IsInitialized()
{
    return bInitialized;
}

bool SteamAPIWrapper::IsLoggedIn()
{
#if STEAM_SDK_AVAILABLE
    if (bInitialized && SteamUser())
    {
        return SteamUser()->BLoggedOn();
    }
    return false;
#else
    return bInitialized; // Placeholder mode
#endif
}

FString SteamAPIWrapper::GetPlayerName()
{
#if STEAM_SDK_AVAILABLE
    if (bInitialized && SteamFriends())
    {
        const char* PlayerName = SteamFriends()->GetPersonaName();
        if (PlayerName)
        {
            return FString(UTF8_TO_TCHAR(PlayerName));
        }
    }
    return TEXT("Unknown Player");
#else
    return TEXT("Placeholder Player");
#endif
}

uint64 SteamAPIWrapper::GetSteamID()
{
#if STEAM_SDK_AVAILABLE
    if (bInitialized && SteamUser())
    {
        CSteamID SteamID = SteamUser()->GetSteamID();
        return SteamID.ConvertToUint64();
    }
    return 0;
#else
    return 12345678901234567; // Placeholder ID
#endif
}
