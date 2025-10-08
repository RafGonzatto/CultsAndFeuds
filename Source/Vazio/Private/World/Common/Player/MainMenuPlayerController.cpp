#include "World/Common/Player/MainMenuPlayerController.h"
#include "Logging/VazioLogFacade.h"
#include "UI/Widgets/MainMenuWidget.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "Blueprint/WidgetLayoutLibrary.h"

AMainMenuPlayerController::AMainMenuPlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
    Menu = nullptr;

    if (!MenuClass)
    {
        MenuClass = UMainMenuWidget::StaticClass();
    }
}

void AMainMenuPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (!IsLocalPlayerController()) return;
    GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AMainMenuPlayerController::SpawnMenu);
}

void AMainMenuPlayerController::SpawnMenu()
{
    if (!IsLocalPlayerController()) return;
    if (Menu) return;
    if (!MenuClass) return;

    Menu = CreateWidget<UUserWidget>(this, MenuClass);
    if (!Menu) return;

    Menu->SetVisibility(ESlateVisibility::Visible);
    Menu->SetIsEnabled(true);
    Menu->AddToPlayerScreen(32767);

    // Determine a focusable target from the widget
    UWidget* FocusTarget = nullptr;
    if (UMainMenuWidget* AsMainMenu = Cast<UMainMenuWidget>(Menu))
    {
        FocusTarget = AsMainMenu->GetInitialFocusTarget();
    }
    if (!FocusTarget)
    {
        FocusTarget = Menu; // fallback
    }

    FInputModeUIOnly Input;
    Input.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    Input.SetWidgetToFocus(FocusTarget->TakeWidget());
    SetInputMode(Input);
    bShowMouseCursor = true;

    FSlateApplication::Get().SetKeyboardFocus(FocusTarget->TakeWidget(), EFocusCause::SetDirectly);

    // Log útil de diagnóstico
    const FVector2D Size = UWidgetLayoutLibrary::GetViewportSize(this);
    const float Scale = UWidgetLayoutLibrary::GetViewportScale(this);
    LOG_UI(Warn, TEXT("[PC] InViewport=%d Vis=%d Opacity=%.2f Size=%.0fx%.0f Scale=%.3f"),
        Menu->IsInViewport(), (int32)Menu->GetVisibility(), Menu->GetRenderOpacity(), Size.X, Size.Y, Scale);
}
