#include "UI/Widgets/BaseModalWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/Engine.h"

UBaseModalWidget::UBaseModalWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true); // permite receber teclas
}

TSharedRef<SWidget> UBaseModalWidget::RebuildWidget()
{
    TSharedPtr<STextBlock> TitleText;

	TSharedRef<SWidget> Root = SNew(SBorder)
		.Padding(20.f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 16)
			[
				SAssignNew(TitleText, STextBlock)
				.Text(GetTitle())
				.Justification(ETextJustify::Center)
			]
            + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,12)
            [
                BuildBody()
            ]
			+ SVerticalBox::Slot().AutoHeight()
			[
                SAssignNew(CloseButton, SButton)
                .IsFocusable(true)
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                .OnClicked_Lambda([this]()
                {
                    UE_LOG(LogTemp, Warning, TEXT("[Modal] CloseButton clicked -> HandleClose()"));
                    HandleClose();
                    return FReply::Handled();
                })
                [
                    SNew(STextBlock).Text(FText::FromString(TEXT("Close")))
                ]
            ]
        ];

    return Root;
}

void UBaseModalWidget::NativeConstruct()
{
    Super::NativeConstruct();
    UE_LOG(LogTemp, Warning, TEXT("[Modal] NativeConstruct -> FocusFirstWidget"));
    FocusFirstWidget();
}

FReply UBaseModalWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    UE_LOG(LogTemp, Warning, TEXT("[Modal] NativeOnKeyDown key=%s"), *InKeyEvent.GetKey().ToString());
    if (InKeyEvent.GetKey() == EKeys::E || InKeyEvent.GetKey() == EKeys::Escape)
    {
        HandleClose();
        return FReply::Handled();
    }
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UBaseModalWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    UE_LOG(LogTemp, Warning, TEXT("[Modal] NativeOnPreviewKeyDown key=%s"), *InKeyEvent.GetKey().ToString());
    if (InKeyEvent.GetKey() == EKeys::E || InKeyEvent.GetKey() == EKeys::Escape)
    {
        HandleClose();
        return FReply::Handled();
    }
    return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

void UBaseModalWidget::HandleClose()
{
    UE_LOG(LogTemp, Warning, TEXT("[Modal] HandleClose -> RemoveFromParent + Broadcast"));
    RemoveFromParent();
    OnModalClosed.Broadcast();
}

void UBaseModalWidget::FocusFirstWidget()
{
    // garanta foco no widget raiz para receber teclas
    FSlateApplication::Get().SetKeyboardFocus(TakeWidget());

    // Se o botao for focavel, tente focar tambem
    if (CloseButton.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Modal] FocusFirstWidget -> focusing CloseButton"));
        FSlateApplication::Get().SetKeyboardFocus(CloseButton);
    }
}

FText UBaseModalWidget::GetTitle() const
{
    return FText::FromString(TEXT("Interacao"));
}

TSharedRef<SWidget> UBaseModalWidget::BuildBody()
{
    return SNew(STextBlock).Text(FText::FromString(TEXT("Conteudo base")));
}

// ---- Derived Modals ----
TSharedRef<SWidget> UHouseModalWidget::BuildBody()
{
    return SNew(STextBlock).Text(FText::FromString(TEXT("Bem-vindo � House")));
}

TSharedRef<SWidget> UArenaModalWidget::BuildBody()
{
    return SNew(STextBlock).Text(FText::FromString(TEXT("Bem-vindo � Arena")));
}

TSharedRef<SWidget> UShopModalWidget::BuildBody()
{
    return SNew(STextBlock).Text(FText::FromString(TEXT("Bem-vindo � Shop")));
}
