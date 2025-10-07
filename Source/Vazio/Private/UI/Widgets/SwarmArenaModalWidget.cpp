#include "UI/Widgets/SwarmArenaModalWidget.h"
// #include "Swarm/SwarmGameFlow.h" // Removed - Swarm system deleted
#include "Kismet/GameplayStatics.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "EngineUtils.h"

TSharedRef<SWidget> USwarmArenaModalWidget::BuildBody()
{
    return SNew(SVerticalBox)
    + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,8)
    [
        SNew(STextBlock).Text(FText::FromString(TEXT("Bem-vindo à Arena")))
    ]
    + SVerticalBox::Slot().AutoHeight()
    [
        SNew(SButton)
        .OnClicked_Lambda([this]()
        {
            // TODO: SwarmGameFlow removed - implement generic level transition
            UE_LOG(LogTemp, Warning, TEXT("[SwarmArenaModal] Enter Arena clicked - SwarmGameFlow removed"));
            
            // Simple level change as fallback
            if (UWorld* World = GetWorld())
            {
                UGameplayStatics::OpenLevel(World, TEXT("Battle_Main"));
            }
            return FReply::Handled();
        })
        [
            SNew(STextBlock).Text(FText::FromString(TEXT("Enter the Arena")))
        ]
    ];
}

