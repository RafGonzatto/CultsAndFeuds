#include "UI/Widgets/UMGPauseMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Framework/Application/SlateApplication.h"
#include "UObject/ConstructorHelpers.h"

// Static member definition
TWeakObjectPtr<UUMGPauseMenuWidget> UUMGPauseMenuWidget::CurrentPauseMenu = nullptr;

UUMGPauseMenuWidget::UUMGPauseMenuWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetIsFocusable(true);
}

void UUMGPauseMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind button events if they exist
    if (ContinueButton)
    {
        ContinueButton->OnClicked.AddDynamic(this, &UUMGPauseMenuWidget::OnContinueClicked);
        UE_LOG(LogTemp, Log, TEXT("[UMGPauseMenu] ContinueButton bound"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMGPauseMenu] ContinueButton not found"));
    }

    if (SettingsButton)
    {
        SettingsButton->OnClicked.AddDynamic(this, &UUMGPauseMenuWidget::OnSettingsClicked);
        UE_LOG(LogTemp, Log, TEXT("[UMGPauseMenu] SettingsButton bound"));
    }

    if (ExitButton)
    {
        ExitButton->OnClicked.AddDynamic(this, &UUMGPauseMenuWidget::OnExitClicked);
        UE_LOG(LogTemp, Log, TEXT("[UMGPauseMenu] ExitButton bound"));
    }

    // Set title text if it exists
    if (TitleText)
    {
        TitleText->SetText(FText::FromString(TEXT("Game Paused")));
    }

    UE_LOG(LogTemp, Log, TEXT("[UMGPauseMenu] NativeConstruct completed"));
}

FReply UUMGPauseMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    // Handle ESC to close pause menu
    if (InKeyEvent.GetKey() == EKeys::Escape)
    {
        UE_LOG(LogTemp, Log, TEXT("[UMGPauseMenu] ESC pressed in pause menu"));
        OnContinueClicked(); // Same as continue
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UUMGPauseMenuWidget::OnContinueClicked()
{
    UE_LOG(LogTemp, Log, TEXT("[UMGPauseMenu] Continue clicked"));
    HidePauseMenu(GetWorld());
}

void UUMGPauseMenuWidget::OnSettingsClicked()
{
    UE_LOG(LogTemp, Log, TEXT("[UMGPauseMenu] Settings clicked - TODO: Implement settings menu"));
    // TODO: Implement settings menu
    // For now, just show a message
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Settings menu not implemented yet"));
    }
}

void UUMGPauseMenuWidget::OnExitClicked()
{
    UE_LOG(LogTemp, Log, TEXT("[UMGPauseMenu] Exit clicked"));
    
    if (UWorld* World = GetWorld())
    {
        // First unpause
        if (AGameModeBase* GameMode = World->GetAuthGameMode())
        {
            GameMode->ClearPause();
        }

        // Then quit to main menu or exit
        UGameplayStatics::OpenLevel(World, FName("MainMenu"));
    }
}

void UUMGPauseMenuWidget::ShowPauseMenu(UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[UMGPauseMenu] ShowPauseMenu: Invalid World"));
        return;
    }

    // Don't create multiple pause menus
    if (CurrentPauseMenu.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMGPauseMenu] Pause menu already visible"));
        return;
    }

    // Get player controller
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("[UMGPauseMenu] ShowPauseMenu: No PlayerController"));
        return;
    }

    // Pause the game
    if (AGameModeBase* GameMode = World->GetAuthGameMode())
    {
        GameMode->SetPause(PC);
        UE_LOG(LogTemp, Log, TEXT("[UMGPauseMenu] Game paused"));
    }

    // Create from C++ class directly (simpler approach)
    if (UUMGPauseMenuWidget* PauseWidget = CreateWidget<UUMGPauseMenuWidget>(PC, UUMGPauseMenuWidget::StaticClass()))
    {
        PauseWidget->AddToViewport(1000); // High Z-order to appear on top
        CurrentPauseMenu = PauseWidget;

        // Set input mode to UI only
        FInputModeUIOnly UIInputMode;
        UIInputMode.SetWidgetToFocus(PauseWidget->TakeWidget());
        PC->SetInputMode(UIInputMode);
        PC->SetShowMouseCursor(true);

        UE_LOG(LogTemp, Log, TEXT("[UMGPauseMenu] C++ pause menu created and shown"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[UMGPauseMenu] Failed to create pause widget"));
    }
}

void UUMGPauseMenuWidget::HidePauseMenu(UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[UMGPauseMenu] HidePauseMenu: Invalid World"));
        return;
    }

    // Remove the pause menu if it exists
    if (CurrentPauseMenu.IsValid())
    {
        CurrentPauseMenu->RemoveFromParent();
        CurrentPauseMenu = nullptr;
        UE_LOG(LogTemp, Log, TEXT("[UMGPauseMenu] Pause menu hidden"));
    }

    // Get player controller
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("[UMGPauseMenu] HidePauseMenu: No PlayerController"));
        return;
    }

    // Unpause the game
    if (AGameModeBase* GameMode = World->GetAuthGameMode())
    {
        GameMode->ClearPause();
        UE_LOG(LogTemp, Log, TEXT("[UMGPauseMenu] Game unpaused"));
    }

    // Restore game input mode
    FInputModeGameOnly GameInputMode;
    PC->SetInputMode(GameInputMode);
    PC->SetShowMouseCursor(true); // Keep mouse cursor for our top-down game

    UE_LOG(LogTemp, Log, TEXT("[UMGPauseMenu] Input mode restored to game"));
}

bool UUMGPauseMenuWidget::IsPauseMenuVisible(UWorld* World)
{
    return CurrentPauseMenu.IsValid();
}
