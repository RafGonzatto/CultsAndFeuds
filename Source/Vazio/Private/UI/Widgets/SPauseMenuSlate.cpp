#include "UI/Widgets/SPauseMenuSlate.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

// Static member definitions
TWeakPtr<SPauseMenuSlate> SPauseMenuSlate::CurrentPauseMenu = nullptr;
TSharedPtr<SWidget> SPauseMenuSlate::CurrentPauseContainer = nullptr;

void SPauseMenuSlate::Construct(const FArguments& InArgs)
{
    ChildSlot
    [
        SNew(SConstraintCanvas)
        + SConstraintCanvas::Slot()
        .Anchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f))
        .Offset(FMargin(-200, -150, 200, 150))
        [
            SNew(SBorder)
            .BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
            .Padding(20.0f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 0, 0, 20)
                .HAlign(HAlign_Center)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("Game Paused")))
                    .Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
                    .ColorAndOpacity(FLinearColor::White)
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 5)
                .HAlign(HAlign_Center)
                [
                    SNew(SBox)
                    .WidthOverride(200.0f)
                    .HeightOverride(40.0f)
                    [
                        SNew(SButton)
                        .Text(FText::FromString(TEXT("Continue")))
                        .HAlign(HAlign_Center)
                        .VAlign(VAlign_Center)
                        .OnClicked(this, &SPauseMenuSlate::OnContinueClicked)
                    ]
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 5)
                .HAlign(HAlign_Center)
                [
                    SNew(SBox)
                    .WidthOverride(200.0f)
                    .HeightOverride(40.0f)
                    [
                        SNew(SButton)
                        .Text(FText::FromString(TEXT("Settings")))
                        .HAlign(HAlign_Center)
                        .VAlign(VAlign_Center)
                        .OnClicked(this, &SPauseMenuSlate::OnSettingsClicked)
                    ]
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 5)
                .HAlign(HAlign_Center)
                [
                    SNew(SBox)
                    .WidthOverride(200.0f)
                    .HeightOverride(40.0f)
                    [
                        SNew(SButton)
                        .Text(FText::FromString(TEXT("Exit to Main Menu")))
                        .HAlign(HAlign_Center)
                        .VAlign(VAlign_Center)
                        .OnClicked(this, &SPauseMenuSlate::OnExitClicked)
                    ]
                ]
            ]
        ]
    ];

    // Set keyboard focus to this widget so it can receive ESC key
    SetCanTick(false);
}

FReply SPauseMenuSlate::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::Escape)
    {
        UE_LOG(LogTemp, Log, TEXT("[SPauseMenuSlate] ESC pressed in pause menu"));
        OnContinueClicked();
        return FReply::Handled();
    }

    return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

FReply SPauseMenuSlate::OnContinueClicked()
{
    UE_LOG(LogTemp, Log, TEXT("[SPauseMenuSlate] Continue clicked"));
    
    // Get world from engine (a bit hacky but works for this case)
    if (UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::LogAndReturnNull) : nullptr)
    {
        HidePauseMenu(World);
    }
    
    return FReply::Handled();
}

FReply SPauseMenuSlate::OnSettingsClicked()
{
    UE_LOG(LogTemp, Log, TEXT("[SPauseMenuSlate] Settings clicked - TODO: Implement settings menu"));
    
    // For now, just show a message
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Settings menu not implemented yet"));
    }
    
    return FReply::Handled();
}

FReply SPauseMenuSlate::OnExitClicked()
{
    UE_LOG(LogTemp, Log, TEXT("[SPauseMenuSlate] Exit clicked"));
    
    // Get world from engine
    if (UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::LogAndReturnNull) : nullptr)
    {
        // First unpause
        if (AGameModeBase* GameMode = World->GetAuthGameMode())
        {
            GameMode->ClearPause();
        }

        // Then quit to main menu
        UGameplayStatics::OpenLevel(World, FName("MainMenu"));
    }
    
    return FReply::Handled();
}

void SPauseMenuSlate::ShowPauseMenu(UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[SPauseMenuSlate] ShowPauseMenu: Invalid World"));
        return;
    }

    // Don't create multiple pause menus
    if (CurrentPauseMenu.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[SPauseMenuSlate] Pause menu already visible"));
        return;
    }

    // Get player controller
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("[SPauseMenuSlate] ShowPauseMenu: No PlayerController"));
        return;
    }

    // Pause the game
    if (AGameModeBase* GameMode = World->GetAuthGameMode())
    {
        GameMode->SetPause(PC);
        UE_LOG(LogTemp, Log, TEXT("[SPauseMenuSlate] Game paused"));
    }

    // Create the pause menu widget
    TSharedRef<SPauseMenuSlate> PauseMenuWidget = SNew(SPauseMenuSlate);
    CurrentPauseMenu = PauseMenuWidget;

    // Add to viewport
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->AddViewportWidgetContent(
            PauseMenuWidget,
            1000 // High Z-order
        );
        
        CurrentPauseContainer = PauseMenuWidget;

        // Set input mode to UI only
        FInputModeUIOnly UIInputMode;
        UIInputMode.SetWidgetToFocus(PauseMenuWidget);
        PC->SetInputMode(UIInputMode);
        PC->SetShowMouseCursor(true);

        // Set keyboard focus
        FSlateApplication::Get().SetKeyboardFocus(PauseMenuWidget);

        UE_LOG(LogTemp, Log, TEXT("[SPauseMenuSlate] Pause menu created and shown"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[SPauseMenuSlate] Failed to get GameViewport"));
    }
}

void SPauseMenuSlate::HidePauseMenu(UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[SPauseMenuSlate] HidePauseMenu: Invalid World"));
        return;
    }

    // Remove the pause menu if it exists
    if (CurrentPauseContainer.IsValid() && GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->RemoveViewportWidgetContent(CurrentPauseContainer.ToSharedRef());
        CurrentPauseContainer.Reset();
        CurrentPauseMenu.Reset();
        UE_LOG(LogTemp, Log, TEXT("[SPauseMenuSlate] Pause menu hidden"));
    }

    // Get player controller
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("[SPauseMenuSlate] HidePauseMenu: No PlayerController"));
        return;
    }

    // Unpause the game
    if (AGameModeBase* GameMode = World->GetAuthGameMode())
    {
        GameMode->ClearPause();
        UE_LOG(LogTemp, Log, TEXT("[SPauseMenuSlate] Game unpaused"));
    }

    // Restore game input mode
    FInputModeGameOnly GameInputMode;
    PC->SetInputMode(GameInputMode);
    PC->SetShowMouseCursor(true); // Keep mouse cursor for our top-down game

    UE_LOG(LogTemp, Log, TEXT("[SPauseMenuSlate] Input mode restored to game"));
}

bool SPauseMenuSlate::IsPauseMenuVisible(UWorld* World)
{
    return CurrentPauseMenu.IsValid();
}
