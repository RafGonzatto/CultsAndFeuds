#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SButton;
class STextBlock;

/**
 * Simple Slate-based pause menu
 */
class VAZIO_API SPauseMenuSlate : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SPauseMenuSlate) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    // Static functions for pause menu management
    static void ShowPauseMenu(UWorld* World);
    static void HidePauseMenu(UWorld* World);
    static bool IsPauseMenuVisible(UWorld* World);

private:
    // Button callbacks
    FReply OnContinueClicked();
    FReply OnSettingsClicked();
    FReply OnExitClicked();

    // Handle ESC key
    virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

    // Static instance tracking
    static TWeakPtr<SPauseMenuSlate> CurrentPauseMenu;
    static TSharedPtr<SWidget> CurrentPauseContainer;
};