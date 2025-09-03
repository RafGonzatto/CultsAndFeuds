#include "UI/Widgets/MainMenuWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Styling/CoreStyle.h"

#include "Core/Flow/FlowSubsystem.h"
#include "Engine/GameInstance.h"

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Root SizeBox (fixed size for simplicity)
    USizeBox* RootSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RootSizeBox"));
    RootSizeBox->SetWidthOverride(1920.0f);
    RootSizeBox->SetHeightOverride(1080.0f);
    WidgetTree->RootWidget = RootSizeBox;

    // Overlay
    UOverlay* MainOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("MainOverlay"));
    RootSizeBox->AddChild(MainOverlay);

    // Fullscreen background
    UBorder* FullScreenBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("FullScreenBorder"));
    FullScreenBorder->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.2f, 0.9f));
    if (UOverlaySlot* BgSlot = MainOverlay->AddChildToOverlay(FullScreenBorder))
    {
        BgSlot->SetHorizontalAlignment(HAlign_Fill);
        BgSlot->SetVerticalAlignment(VAlign_Fill);
    }

    // Title text
    UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
    TitleText->SetText(FText::FromString(TEXT("MAIN MENU")));
    FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 48);
    TitleText->SetFont(TitleFont);
    TitleText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    TitleText->SetJustification(ETextJustify::Center);
    if (UOverlaySlot* TextSlot = MainOverlay->AddChildToOverlay(TitleText))
    {
        TextSlot->SetHorizontalAlignment(HAlign_Center);
        TextSlot->SetVerticalAlignment(VAlign_Top);
        TextSlot->SetPadding(FMargin(0.0f, 100.0f, 0.0f, 0.0f));
    }

    // Menu box
    MenuBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MenuBox"));
    if (UOverlaySlot* MenuSlot = MainOverlay->AddChildToOverlay(MenuBox))
    {
        MenuSlot->SetHorizontalAlignment(HAlign_Center);
        MenuSlot->SetVerticalAlignment(VAlign_Center);
    }

    // Play button
    CreatePlayButton();

    // Force visibility
    this->SetVisibility(ESlateVisibility::Visible);
    RootSizeBox->SetVisibility(ESlateVisibility::Visible);
    MainOverlay->SetVisibility(ESlateVisibility::Visible);
    FullScreenBorder->SetVisibility(ESlateVisibility::Visible);

    UE_LOG(LogTemp, Warning, TEXT("[MainMenuWidget] Widget criado"));
}

UButton* UMainMenuWidget::CreatePlayButton()
{
    USizeBox* ButtonSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("ButtonSizeBox"));
    ButtonSizeBox->SetWidthOverride(300.0f);
    ButtonSizeBox->SetHeightOverride(80.0f);

    UBorder* ButtonBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ButtonBorder"));
    ButtonBorder->SetBrushColor(FLinearColor(0.2f, 0.6f, 0.2f, 1.0f));
    ButtonBorder->SetPadding(FMargin(10.0f));
    ButtonSizeBox->AddChild(ButtonBorder);

    UButton* PlayBtn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("PlayButton"));
    ButtonBorder->SetContent(PlayBtn);

    UTextBlock* PlayTxt = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PlayText"));
    PlayTxt->SetText(FText::FromString(TEXT("PLAY GAME")));
    FSlateFontInfo ButtonFont = FCoreStyle::GetDefaultFontStyle("Regular", 24);
    PlayTxt->SetFont(ButtonFont);
    PlayTxt->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    PlayTxt->SetJustification(ETextJustify::Center);
    PlayBtn->SetContent(PlayTxt);

    if (UVerticalBoxSlot* VBSlot = MenuBox->AddChildToVerticalBox(ButtonSizeBox))
    {
        VBSlot->SetPadding(FMargin(0.0f, 20.0f, 0.0f, 0.0f));
        VBSlot->SetHorizontalAlignment(HAlign_Center);
        VBSlot->SetVerticalAlignment(VAlign_Center);
    }

    PlayBtn->OnClicked.AddDynamic(this, &UMainMenuWidget::HandlePlay);
    UE_LOG(LogTemp, Warning, TEXT("[MainMenuWidget] Botao Play criado"));
    return PlayBtn;
}

void UMainMenuWidget::HandlePlay()
{
    UE_LOG(LogTemp, Warning, TEXT("[MainMenuWidget] Botao PLAY clicado"));

    UGameInstance* GI = GetGameInstance();
    if (!GI)
    {
        UE_LOG(LogTemp, Error, TEXT("[MainMenuWidget] GameInstance e NULL!"));
        return;
    }

    UFlowSubsystem* Flow = GI->GetSubsystem<UFlowSubsystem>();
    if (!Flow)
    {
        UE_LOG(LogTemp, Error, TEXT("[MainMenuWidget] FlowSubsystem NAO ENCONTRADO!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[MainMenuWidget] Chamando OpenCity()..."));
    Flow->OpenCity();
}

