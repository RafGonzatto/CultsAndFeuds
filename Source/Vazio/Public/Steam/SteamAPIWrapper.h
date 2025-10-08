#pragma once

#include "CoreMinimal.h"

#if STEAM_SDK_AVAILABLE
PRAGMA_DISABLE_DEPRECATION_WARNINGS
#include "steam_api.h"
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#endif

/**
 * Steam API wrapper for safe initialization and cleanup
 */
// Forward declaration
struct FSteamFriend;

class VAZIO_API SteamAPIWrapper
{
public:
    static bool Initialize();
    static void Shutdown();
    static bool IsInitialized();
    static bool IsLoggedIn();
    static FString GetPlayerName();
    static uint64 GetSteamID();
    
    // Try to get real Steam friends even in development mode
    static bool TryGetRealSteamFriends(TArray<struct FSteamFriend>& OutFriends);
    
private:
    static bool bInitialized;
};
