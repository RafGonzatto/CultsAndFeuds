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
#include "Engine/Engine.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Criar um SizeBox como root - garante tamanho
	USizeBox* RootSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RootSizeBox"));
	RootSizeBox->SetWidthOverride(1920.0f);  // Tamanho fixo
	RootSizeBox->SetHeightOverride(1080.0f); // Tamanho fixo
	WidgetTree->RootWidget = RootSizeBox;

	// Overlay dentro do SizeBox
	UOverlay* MainOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("MainOverlay"));
	RootSizeBox->AddChild(MainOverlay);

	// FUNDO COLORIDO COMPLETO - deve ser visível
	UBorder* FullScreenBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("FullScreenBorder"));
	FullScreenBorder->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.2f, 0.9f)); // Azul escuro mais suave
	
	if (UOverlaySlot* BgSlot = MainOverlay->AddChildToOverlay(FullScreenBorder))
	{
		BgSlot->SetHorizontalAlignment(HAlign_Fill);
		BgSlot->SetVerticalAlignment(VAlign_Fill);
	}

	// TEXTO DE TÍTULO com fonte correta
	UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
	TitleText->SetText(FText::FromString(TEXT("MAIN MENU")));
	
	// Configurar fonte usando fonte padrão do engine
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

	// Container para botões
	MenuBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MenuBox"));
	
	if (UOverlaySlot* MenuSlot = MainOverlay->AddChildToOverlay(MenuBox))
	{
		MenuSlot->SetHorizontalAlignment(HAlign_Center);
		MenuSlot->SetVerticalAlignment(VAlign_Center);
	}

	// Botão gigante
	CreatePlayButton();

	// FORÇAR VISIBILIDADE EXPLÍCITA
	this->SetVisibility(ESlateVisibility::Visible);
	RootSizeBox->SetVisibility(ESlateVisibility::Visible);
	MainOverlay->SetVisibility(ESlateVisibility::Visible);
	FullScreenBorder->SetVisibility(ESlateVisibility::Visible);

	UE_LOG(LogTemp, Warning, TEXT("[MainMenuWidget] Widget criado com fontes corrigidas"));
}

UButton* UMainMenuWidget::CreatePlayButton()
{
	// SizeBox para o botão (tamanho fixo)
	USizeBox* ButtonSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("ButtonSizeBox"));
	ButtonSizeBox->SetWidthOverride(300.0f); 
	ButtonSizeBox->SetHeightOverride(80.0f); 

	// Border para visual do botão
	UBorder* ButtonBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ButtonBorder"));
	ButtonBorder->SetBrushColor(FLinearColor(0.2f, 0.6f, 0.2f, 1.0f)); // Verde mais suave
	ButtonBorder->SetPadding(FMargin(10.0f));
	ButtonSizeBox->AddChild(ButtonBorder);

	// Botão real
	UButton* PlayBtn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("PlayButton"));
	ButtonBorder->SetContent(PlayBtn);
	
	// Texto do botão com fonte correta
	UTextBlock* PlayTxt = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PlayText"));
	PlayTxt->SetText(FText::FromString(TEXT("PLAY GAME")));
	
	// Usar fonte padrão do engine
	FSlateFontInfo ButtonFont = FCoreStyle::GetDefaultFontStyle("Regular", 24);
	PlayTxt->SetFont(ButtonFont);
	PlayTxt->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	PlayTxt->SetJustification(ETextJustify::Center);

	PlayBtn->SetContent(PlayTxt);

	// Adicionar ao menu
	if (UVerticalBoxSlot* VBSlot = MenuBox->AddChildToVerticalBox(ButtonSizeBox))
	{
		VBSlot->SetPadding(FMargin(0.0f, 20.0f, 0.0f, 0.0f));
		VBSlot->SetHorizontalAlignment(HAlign_Center);
		VBSlot->SetVerticalAlignment(VAlign_Center);
	}

	// Conectar evento
	PlayBtn->OnClicked.AddDynamic(this, &UMainMenuWidget::HandlePlay);

	UE_LOG(LogTemp, Warning, TEXT("[MainMenuWidget] Botão criado com fonte padrão do engine"));
	
	return PlayBtn;
}

void UMainMenuWidget::HandlePlay()
{
	UE_LOG(LogTemp, Error, TEXT("[MainMenuWidget] *** BOTÃO PLAY CLICADO! ***"));
	
	// Mensagem na tela
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("*** BOTÃO PLAY CLICADO! ***"));
	}
	
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainMenuWidget] GameInstance é NULL!"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ERRO: GameInstance NULL"));
		}
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[MainMenuWidget] GameInstance encontrado, buscando FlowSubsystem..."));
	
	UFlowSubsystem* Flow = GI->GetSubsystem<UFlowSubsystem>();
	if (!Flow)
	{
		UE_LOG(LogTemp, Error, TEXT("[MainMenuWidget] FlowSubsystem NÃO ENCONTRADO!"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ERRO: FlowSubsystem não encontrado!"));
		}
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[MainMenuWidget] FlowSubsystem encontrado! Chamando OpenCity()..."));
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("FlowSubsystem encontrado! Indo para cidade..."));
	}
	
	Flow->OpenCity();
}
