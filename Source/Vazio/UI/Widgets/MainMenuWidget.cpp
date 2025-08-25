#include "UI/Widgets/MainMenuWidget.h"      // 1º include SEMPRE o .h da classe
#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

#include "Core/Flow/FlowSubsystem.h"
#include "Engine/GameInstance.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Construção programática do layout
	WidgetTree->RootWidget = RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));

	MenuBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MenuBox"));
	if (UOverlaySlot* OverlaySlot = RootOverlay->AddChildToOverlay(MenuBox))
	{
		OverlaySlot->SetHorizontalAlignment(HAlign_Center);
		OverlaySlot->SetVerticalAlignment(VAlign_Center);
	}

	// Botão Play
	CreateButton(TEXT("Play"), [this]()
		{
			HandlePlay();
		});
}

UButton* UMainMenuWidget::CreateButton(const FString& Label, TFunction<void()> OnClickedLambda)
{
	UButton* Btn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	UTextBlock* Txt = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());

	Txt->SetText(FText::FromString(Label));
	Btn->AddChild(Txt);

	if (UVerticalBoxSlot* VBSlot = MenuBox->AddChildToVerticalBox(Btn))
	{
		VBSlot->SetPadding(FMargin(8.f));
		VBSlot->SetHorizontalAlignment(HAlign_Center);
	}

	// Bridge lambda → método UFUNCTION (garante compatibilidade com AddDynamic)
	// Aqui criamos um wrapper local que chama o lambda:
	class FClickProxy : public UObject
	{
		GENERATED_BODY()
	public:
		TFunction<void()> Click;
		UFUNCTION() void Thunk() { if (Click) Click(); }
	};
	FClickProxy* Proxy = NewObject<FClickProxy>(this);
	Proxy->Click = MoveTemp(OnClickedLambda);
	Btn->OnClicked.AddDynamic(Proxy, &FClickProxy::Thunk);

	return Btn;
}

void UMainMenuWidget::HandlePlay()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UFlowSubsystem* Flow = GI->GetSubsystem<UFlowSubsystem>())
		{
			Flow->OpenCity();
			return;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("[Menu] FlowSubsystem não encontrado"));
}
