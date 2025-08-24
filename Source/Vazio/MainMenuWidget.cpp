// MainMenuWidget.cpp
#include "MainMenuWidget.h"                 // 1�: sempre o pr�prio header
#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"

UMainMenuWidget::UMainMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// O valor � lido s� na constru��o; aqui est� ok mesmo com aviso deprecado.
	bIsFocusable = true;
}

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UE_LOG(LogTemp, Warning, TEXT("[Menu] NativeConstruct"));
}

TSharedRef<SWidget> UMainMenuWidget::RebuildWidget()
{
	// Garante WidgetTree
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, UWidgetTree::StaticClass());
	}

	// Root Overlay
	RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));

	// Caixa vertical para os botões
	MenuBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MenuBox"));

	// Adiciona a MenuBox na Overlay
	if (RootOverlay && MenuBox)
	{
		if (UOverlaySlot* OverlaySlot = RootOverlay->AddChildToOverlay(MenuBox))
		{
			OverlaySlot->SetHorizontalAlignment(HAlign_Center);
			OverlaySlot->SetVerticalAlignment(VAlign_Center);
		}
	}

	// Cria botões
	AddButton(FText::FromString(TEXT("Play")), FName("HandlePlay"));
	AddButton(FText::FromString(TEXT("Continue")), FName("HandleContinue"));
	AddButton(FText::FromString(TEXT("Settings")), FName("HandleSettings"));
	AddButton(FText::FromString(TEXT("Exit")), FName("HandleExit"));

	// Define a raiz da árvore ANTES de chamar Super
	WidgetTree->RootWidget = RootOverlay;

	UE_LOG(LogTemp, Warning, TEXT("[Menu] UI construída (RebuildWidget)"));

	return Super::RebuildWidget();
}

void UMainMenuWidget::AddButton(const FText& Label, FName HandlerName)
{
	if (!MenuBox || !WidgetTree) return;

	// Cria bot�o e texto
	UButton* Btn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	UTextBlock* Txt = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Txt->SetText(Label);
	Txt->SetJustification(ETextJustify::Center);

	// Anexa texto ao bot�o
	Btn->AddChild(Txt);

	// Enfileira no VerticalBox
	if (UVerticalBoxSlot* VSlot = MenuBox->AddChildToVerticalBox(Btn))
	{
		VSlot->SetPadding(FMargin(8.f));
		VSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	// Bind do clique por nome via FScriptDelegate (dinâmico)
	if (HandlerName != NAME_None)
	{
		FScriptDelegate ClickDelegate;
		ClickDelegate.BindUFunction(this, HandlerName);
		Btn->OnClicked.Add(ClickDelegate);
	}

	UE_LOG(LogTemp, Warning, TEXT("[Menu] Bot�o criado: %s"), *Label.ToString());
}

void UMainMenuWidget::HandlePlay()
{
	UE_LOG(LogTemp, Warning, TEXT("[Menu] Play"));
}

void UMainMenuWidget::HandleContinue()
{
	UE_LOG(LogTemp, Warning, TEXT("[Menu] Continue"));
}

void UMainMenuWidget::HandleSettings()
{
	UE_LOG(LogTemp, Warning, TEXT("[Menu] Settings"));
}

void UMainMenuWidget::HandleExit()
{
	UE_LOG(LogTemp, Warning, TEXT("[Menu] Exit"));
	if (UWorld* World = GetWorld())
	{
		UKismetSystemLibrary::QuitGame(World, nullptr, EQuitPreference::Quit, false);
	}
}
