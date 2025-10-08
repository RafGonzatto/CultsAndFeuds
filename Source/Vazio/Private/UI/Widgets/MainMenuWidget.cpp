#include "UI/Widgets/MainMenuWidget.h"
#include "Core/Flow/FlowSubsystem.h"
#include "Components/OverlaySlot.h"
#include "Components/VerticalBoxSlot.h"
#include "Logging/VazioLogFacade.h"

UMainMenuWidget::UMainMenuWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetIsFocusable(true); // Allow the UUserWidget to receive keyboard focus
}

TSharedRef<SWidget> UMainMenuWidget::RebuildWidget()
{
    if (!WidgetTree)
    {
        WidgetTree = NewObject<UWidgetTree>(this, UWidgetTree::StaticClass(), NAME_None, RF_Transactional);
    }

    RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));
    RootOverlay->SetVisibility(ESlateVisibility::Visible);
    RootOverlay->SetIsEnabled(true);
    WidgetTree->RootWidget = RootOverlay;

    UBorder* FullScreenBG = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BG"));
    FullScreenBG->SetBrushColor(FLinearColor(0.05f, 0.05f, 0.1f, 0.85f));
    UOverlaySlot* BgSlot = Cast<UOverlaySlot>(RootOverlay->AddChildToOverlay(FullScreenBG));
    if (BgSlot)
    {
        BgSlot->SetHorizontalAlignment(HAlign_Fill);
        BgSlot->SetVerticalAlignment(VAlign_Fill);
    }

    UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Title"));
    Title->SetText(FText::FromString(TEXT("MAIN MENU")));
    Title->SetJustification(ETextJustify::Center);
    Title->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 48));
    UOverlaySlot* TitleSlot = Cast<UOverlaySlot>(RootOverlay->AddChildToOverlay(Title));
    if (TitleSlot)
    {
        TitleSlot->SetHorizontalAlignment(HAlign_Center);
        TitleSlot->SetVerticalAlignment(VAlign_Top);
        TitleSlot->SetPadding(FMargin(0, 80, 0, 0));
    }

    MenuBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MenuBox"));
    UOverlaySlot* MenuSlot = Cast<UOverlaySlot>(RootOverlay->AddChildToOverlay(MenuBox));
    if (MenuSlot)
    {
        MenuSlot->SetHorizontalAlignment(HAlign_Center);
        MenuSlot->SetVerticalAlignment(VAlign_Center);
    }

    PlayButton = CreatePlayButton();

    return Super::RebuildWidget();
}

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();
    LOG_UI(Info, TEXT("[MainMenuWidget] Constructed"));
    if (PlayButton)
    {
        PlayButton->SetKeyboardFocus();
    LOG_UI(Debug, TEXT("[MainMenuWidget] Play button focused"));
    }
}

UWidget* UMainMenuWidget::GetInitialFocusTarget() const
{
    return const_cast<UMainMenuWidget*>(this);
}

UButton* UMainMenuWidget::CreatePlayButton()
{
    USizeBox* SB = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("PlaySize"));
    SB->SetWidthOverride(320.f);
    SB->SetHeightOverride(90.f);

    UBorder* B = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PlayBorder"));
    B->SetBrushColor(FLinearColor(0.20f, 0.60f, 0.20f, 1.0f));
    B->SetPadding(FMargin(8.f));
    SB->AddChild(B);

    UButton* Btn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("PlayButton"));
    Btn->SetIsEnabled(true);
    B->SetContent(Btn);

    UTextBlock* Txt = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PlayText"));
    Txt->SetText(FText::FromString(TEXT("PLAY")));
    Txt->SetJustification(ETextJustify::Center);
    Txt->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 28));
    Btn->SetContent(Txt);

    UVerticalBoxSlot* MenuItemSlot = Cast<UVerticalBoxSlot>(MenuBox->AddChildToVerticalBox(SB));
    if (MenuItemSlot)
    {
        MenuItemSlot->SetPadding(FMargin(0, 16, 0, 0));
        MenuItemSlot->SetHorizontalAlignment(HAlign_Center);
        MenuItemSlot->SetVerticalAlignment(VAlign_Center);
    }

    Btn->OnClicked.AddDynamic(this, &UMainMenuWidget::HandlePlay);
    return Btn;
}


void UMainMenuWidget::HandlePlay()
{
    LOG_UI(Info, TEXT("[MainMenuWidget] Botao PLAY clicado"));

    UGameInstance* GI = GetGameInstance();
    if (!GI)
    {
    LOG_UI(Error, TEXT("[MainMenuWidget] GameInstance e NULL!"));
        return;
    }

    UFlowSubsystem* Flow = GI->GetSubsystem<UFlowSubsystem>();
    if (!Flow)
    {
    LOG_UI(Error, TEXT("[MainMenuWidget] FlowSubsystem NAO ENCONTRADO!"));
        return;
    }

    LOG_UI(Info, TEXT("[MainMenuWidget] Chamando OpenCity()..."));
    Flow->OpenCity();
}

