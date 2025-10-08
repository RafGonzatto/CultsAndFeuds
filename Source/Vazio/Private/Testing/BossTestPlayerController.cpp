#include "Testing/BossTestPlayerController.h"
#include "Testing/BossAutoTestSubsystem.h"
#include "Engine/World.h"
#include "Components/InputComponent.h"
#include "Engine/Engine.h"
#include "Logging/VazioLogFacade.h"

ABossTestPlayerController::ABossTestPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ABossTestPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    GetBossTestSubsystem();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, 
            TEXT("Boss Test Controller Active - Press F12 to enable boss testing mode"));
    }
    
    LOG_ENEMIES(Info, TEXT("BossTestPlayerController ready - F12 to activate"));
}

void ABossTestPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    
    if (InputComponent)
    {
        // Bind das teclas numéricas para teste dos bosses
        InputComponent->BindKey(FKey("One"), IE_Pressed, this, &ABossTestPlayerController::OnTestKey1);
        InputComponent->BindKey(FKey("Two"), IE_Pressed, this, &ABossTestPlayerController::OnTestKey2);
        InputComponent->BindKey(FKey("Three"), IE_Pressed, this, &ABossTestPlayerController::OnTestKey3);
        InputComponent->BindKey(FKey("Four"), IE_Pressed, this, &ABossTestPlayerController::OnTestKey4);
        InputComponent->BindKey(FKey("Five"), IE_Pressed, this, &ABossTestPlayerController::OnTestKey5);
        InputComponent->BindKey(FKey("Zero"), IE_Pressed, this, &ABossTestPlayerController::OnTestKey0);
        
        // F12 para ativar/desativar modo teste (changed from F1 due to AMD GPU crashes)
        InputComponent->BindKey(FKey("F12"), IE_Pressed, this, &ABossTestPlayerController::OnTestKeyF1);
        
    LOG_ENEMIES(Info, TEXT("Boss test input bindings setup complete"));
    }
}

void ABossTestPlayerController::OnTestKey1()
{
    GetBossTestSubsystem();
    if (BossTestSubsystem)
    {
        BossTestSubsystem->OnKey1Pressed();
    }
}

void ABossTestPlayerController::OnTestKey2()
{
    GetBossTestSubsystem();
    if (BossTestSubsystem)
    {
        BossTestSubsystem->OnKey2Pressed();
    }
}

void ABossTestPlayerController::OnTestKey3()
{
    GetBossTestSubsystem();
    if (BossTestSubsystem)
    {
        BossTestSubsystem->OnKey3Pressed();
    }
}

void ABossTestPlayerController::OnTestKey4()
{
    GetBossTestSubsystem();
    if (BossTestSubsystem)
    {
        BossTestSubsystem->OnKey4Pressed();
    }
}

void ABossTestPlayerController::OnTestKey5()
{
    GetBossTestSubsystem();
    if (BossTestSubsystem)
    {
        BossTestSubsystem->OnKey5Pressed();
    }
}

void ABossTestPlayerController::OnTestKey0()
{
    GetBossTestSubsystem();
    if (BossTestSubsystem)
    {
        BossTestSubsystem->OnKey0Pressed();
    }
}

void ABossTestPlayerController::OnTestKeyF1()
{
    GetBossTestSubsystem();
    if (BossTestSubsystem)
    {
        // Toggle do modo de teste
        BossTestSubsystem->StartAutoTest();
    }
}

void ABossTestPlayerController::GetBossTestSubsystem()
{
    if (!BossTestSubsystem && GetWorld())
    {
        BossTestSubsystem = GetWorld()->GetSubsystem<UBossAutoTestSubsystem>();
        if (!BossTestSubsystem)
        {
            LOG_ENEMIES(Error, TEXT("Could not find BossAutoTestSubsystem"));
        }
    }
}