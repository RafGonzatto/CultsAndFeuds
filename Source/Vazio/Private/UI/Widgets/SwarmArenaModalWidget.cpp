#include "UI/Widgets/SwarmArenaModalWidget.h"
#include "Swarm/SwarmGameFlow.h"
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
            UWorld* World = GetWorld();
            if (!World) { UE_LOG(LogTemp, Error, TEXT("[SwarmArenaModal] World null")); return FReply::Handled(); }
            UGameInstance* GI = GetGameInstance();
            if (!GI) { UE_LOG(LogTemp, Error, TEXT("[SwarmArenaModal] GameInstance null")); return FReply::Handled(); }
            USwarmGameFlow* Flow = GI->GetSubsystem<USwarmGameFlow>();
            if (!Flow) { UE_LOG(LogTemp, Error, TEXT("[SwarmArenaModal] SwarmGameFlow not found")); return FReply::Handled(); }

            // Find return marker
            FTransform ReturnXf;
            bool bFound = false;
            for (TActorIterator<AActor> It(World); It; ++It)
            {
                AActor* A = *It;
                if (A->ActorHasTag(TEXT("ArenaReturn")) || A->GetName().Contains(TEXT("ArenaReturn")))
                {
                    ReturnXf = A->GetActorTransform();
                    bFound = true;
                    break;
                }
            }
            if (!bFound)
            {
                if (APlayerController* PC = World->GetFirstPlayerController())
                {
                    if (APawn* P = PC->GetPawn())
                    {
                        ReturnXf = P->GetActorTransform();
                        UE_LOG(LogTemp, Warning, TEXT("[SwarmArenaModal] 'ArenaReturn' not found. Using player transform."));
                    }
                }
            }

            const FName CurrentLevelName = FName(*UGameplayStatics::GetCurrentLevelName(World, true));
            Flow->SetReturnPoint(CurrentLevelName, ReturnXf);
            Flow->EnterArena(TEXT("Battle_Main"));
            return FReply::Handled();
        })
        [
            SNew(STextBlock).Text(FText::FromString(TEXT("Enter the Arena")))
        ]
    ];
}

