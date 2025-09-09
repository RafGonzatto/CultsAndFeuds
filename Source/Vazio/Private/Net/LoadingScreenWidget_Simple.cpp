#include "Net/LoadingScreenWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBorder.h"
#include "Engine/Engine.h"

ULoadingScreenWidget::ULoadingScreenWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentLoadingText(TEXT("Loading..."))
{
}

void ULoadingScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Set up input mode for loading screen
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
}

void ULoadingScreenWidget::SetLoadingText(const FString& Text)
{
	CurrentLoadingText = Text;
	if (LoadingTextBlock.IsValid())
	{
		LoadingTextBlock->SetText(FText::FromString(CurrentLoadingText));
	}
}

void ULoadingScreenWidget::ShowLoadingScreen()
{
	SetVisibility(ESlateVisibility::Visible);
	UE_LOG(LogTemp, Warning, TEXT("[LoadingScreen] Showing loading screen: %s"), *CurrentLoadingText);
}

void ULoadingScreenWidget::HideLoadingScreen()
{
	SetVisibility(ESlateVisibility::Collapsed);
	
	// Restore input mode
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[LoadingScreen] Hiding loading screen"));
}

FText ULoadingScreenWidget::GetLoadingText() const
{
	return FText::FromString(CurrentLoadingText);
}

TSharedRef<SWidget> ULoadingScreenWidget::RebuildWidget()
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("BlackBrush"))
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SAssignNew(LoadingTextBlock, STextBlock)
			.Text(FText::FromString(CurrentLoadingText))
			.ColorAndOpacity(FLinearColor::White)
		];
}
