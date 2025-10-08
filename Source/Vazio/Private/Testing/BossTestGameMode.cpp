#include "Testing/BossTestGameMode.h"
#include "Testing/BossTestPlayerController.h"
#include "Testing/BossAutoTestSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Logging/VazioLogFacade.h"

ABossTestGameMode::ABossTestGameMode()
{
    // Definir o PlayerController customizado para testes
    PlayerControllerClass = ABossTestPlayerController::StaticClass();
    
    LOG_ENEMIES(Info, TEXT("BossTestGameMode initialized with BossTestPlayerController"));
}

void ABossTestGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    
    LOG_ENEMIES(Info, TEXT("BossTestGameMode: InitGame called for map: %s"), *MapName);
}

void ABossTestGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    SetupBossTestingEnvironment();
}

void ABossTestGameMode::SetupBossTestingEnvironment()
{
    // Verificar se o BossAutoTestSubsystem está disponível
    if (UBossAutoTestSubsystem* BossTestSystem = GetWorld()->GetSubsystem<UBossAutoTestSubsystem>())
    {
    LOG_ENEMIES(Info, TEXT("Boss testing environment setup complete"));
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, 
                TEXT("=== BOSS TESTING MODE ACTIVE ==="));
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, 
                TEXT("F12 = Enable/Disable Testing"));
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, 
                TEXT("1 = Burrower Boss"));
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, 
                TEXT("2 = Void Queen Boss"));
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, 
                TEXT("3 = Fallen Warlord Boss"));
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, 
                TEXT("4 = Hybrid Demon Boss"));
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, 
                TEXT("5 = All Bosses in Sequence"));
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, 
                TEXT("0 = Stop Current Boss"));
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, 
                TEXT("Console Commands: SpawnBurrower, SpawnAllBosses, etc."));
        }
    }
    else
    {
    LOG_ENEMIES(Error, TEXT("Failed to find BossAutoTestSubsystem"));
    }
}