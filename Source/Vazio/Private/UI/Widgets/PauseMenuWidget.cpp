#include "UI/Widgets/PauseMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Framework/Application/SlateApplication.h"

// Static member definition
TWeakObjectPtr<UPauseMenuWidget> UPauseMenuWidget::CurrentPauseMenu = nullptr;

UPauseMenuWidget::UPauseMenuWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetIsFocusable(true);
}

void UPauseMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind button events
    if (ContinueButton)
    {
        ContinueButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnContinueClicked);
    }

    if (SettingsButton)
    {
        SettingsButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnSettingsClicked);
    }

    if (ExitButton)
    {
        ExitButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnExitClicked);
    }

    // Set title text
    if (TitleText)
    {
        TitleText->SetText(FText::FromString(TEXT("Game Paused")));
    }

    // Focus the continue button by default
    if (ContinueButton)
    {
        FSlateApplication::Get().SetKeyboardFocus(ContinueButton->GetCachedWidget());
    }

    UE_LOG(LogTemp, Log, TEXT("[PauseMenu] Constructed and focused"));
}

FReply UPauseMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    // Handle ESC to close pause menu
    if (InKeyEvent.GetKey() == EKeys::Escape)
    {
        OnContinueClicked(); // Same as continue
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UPauseMenuWidget::OnContinueClicked()
{
    UE_LOG(LogTemp, Log, TEXT("[PauseMenu] Continue clicked"));
    HidePauseMenu(GetWorld());
}

void UPauseMenuWidget::OnSettingsClicked()
{
    UE_LOG(LogTemp, Log, TEXT("[PauseMenu] Settings clicked - TODO: Implement settings menu"));
    // TODO: Implement settings menu
    // For now, just show a message
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Settings menu not implemented yet"));
    }
}

void UPauseMenuWidget::OnExitClicked()
{
    UE_LOG(LogTemp, Log, TEXT("[PauseMenu] Exit clicked"));
    
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

void UPauseMenuWidget::ShowPauseMenu(UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[PauseMenu] ShowPauseMenu: Invalid World"));
        return;
    }

    // Don't create multiple pause menus
    if (CurrentPauseMenu.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[PauseMenu] Pause menu already visible"));
        return;
    }

    // Get player controller
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("[PauseMenu] ShowPauseMenu: No PlayerController"));
        return;
    }

    // Pause the game
    if (AGameModeBase* GameMode = World->GetAuthGameMode())
    {
        GameMode->SetPause(PC);
        UE_LOG(LogTemp, Log, TEXT("[PauseMenu] Game paused"));
    }

    // Create the pause menu widget
    if (UPauseMenuWidget* PauseWidget = CreateWidget<UPauseMenuWidget>(PC, UPauseMenuWidget::StaticClass()))
    {
        PauseWidget->AddToViewport(1000); // High Z-order to appear on top
        CurrentPauseMenu = PauseWidget;

        // Set input mode to UI only
        FInputModeUIOnly UIInputMode;
        UIInputMode.SetWidgetToFocus(PauseWidget->TakeWidget());
        PC->SetInputMode(UIInputMode);
        PC->SetShowMouseCursor(true);

        UE_LOG(LogTemp, Log, TEXT("[PauseMenu] Pause menu created and shown"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[PauseMenu] Failed to create pause widget"));
    }
}

void UPauseMenuWidget::HidePauseMenu(UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[PauseMenu] HidePauseMenu: Invalid World"));
        return;
    }

    // Remove the pause menu if it exists
    if (CurrentPauseMenu.IsValid())
    {
        CurrentPauseMenu->RemoveFromParent();
        CurrentPauseMenu = nullptr;
        UE_LOG(LogTemp, Log, TEXT("[PauseMenu] Pause menu hidden"));
    }

    // Get player controller
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("[PauseMenu] HidePauseMenu: No PlayerController"));
        return;
    }

    // Unpause the game
    if (AGameModeBase* GameMode = World->GetAuthGameMode())
    {
        GameMode->ClearPause();
        UE_LOG(LogTemp, Log, TEXT("[PauseMenu] Game unpaused"));
    }

    // Restore game input mode
    FInputModeGameOnly GameInputMode;
    PC->SetInputMode(GameInputMode);
    PC->SetShowMouseCursor(true); // Keep mouse cursor for our top-down game

    UE_LOG(LogTemp, Log, TEXT("[PauseMenu] Input mode restored to game"));
}

bool UPauseMenuWidget::IsPauseMenuVisible(UWorld* World)
{
    return CurrentPauseMenu.IsValid();
}
