#include "MainMenuGameMode.h"

#include "MainMenuWidget.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/DefaultPawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"

AMainMenuGameMode::AMainMenuGameMode()
{
	// Evita erro de SpawnActor e mant�m um Pawn simples
	DefaultPawnClass = ADefaultPawn::StaticClass();
	bStartPlayersAsSpectators = true;

	UE_LOG(LogTemp, Warning, TEXT("[GM] Ctor"));
}

void AMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		// Prova visual imediata na tela
		UKismetSystemLibrary::PrintString(World, TEXT("[GM] BeginPlay"), true, true, FLinearColor::Green, 6.f);
	}
	UE_LOG(LogTemp, Warning, TEXT("[GM] BeginPlay"));

	// Chama SpawnMenu no pr�ximo tick para garantir viewport pronto
	if (UWorld* World = GetWorld())
	{
		FTimerHandle Th;
		World->GetTimerManager().SetTimerForNextTick(this, &AMainMenuGameMode::SpawnMenu);
	}
}

void AMainMenuGameMode::SpawnMenu()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[GM] SpawnMenu: World nulo"));
		return;
	}

	// Mensagem na tela confirmando que chegamos aqui
	UKismetSystemLibrary::PrintString(World, TEXT("[GM] SpawnMenu()"), true, true, FLinearColor::Yellow, 6.f);

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("[GM] SpawnMenu: PlayerController nulo"));
		UKismetSystemLibrary::PrintString(World, TEXT("[GM] PC nulo"), true, true, FLinearColor::Red, 6.f);
		return;
	}

	Menu = CreateWidget<UMainMenuWidget>(PC, UMainMenuWidget::StaticClass());
	if (!Menu)
	{
		UE_LOG(LogTemp, Error, TEXT("[GM] SpawnMenu: falha ao criar Menu"));
		UKismetSystemLibrary::PrintString(World, TEXT("[GM] Falha ao criar Menu"), true, true, FLinearColor::Red, 6.f);
		return;
	}

	Menu->SetVisibility(ESlateVisibility::Visible);

	// Add to player screen (suficiente)
	const bool bAdded = Menu->AddToPlayerScreen(1000);
	UE_LOG(LogTemp, Warning, TEXT("[GM] AddToPlayerScreen: %s"), bAdded ? TEXT("true") : TEXT("false"));
	UKismetSystemLibrary::PrintString(World, FString::Printf(TEXT("[GM] AddToPlayerScreen: %s"), bAdded ? TEXT("true") : TEXT("false")), true, true, FLinearColor::Green, 6.f);

	// Foco e cursor
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(Menu->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = true;

	UE_LOG(LogTemp, Warning, TEXT("[GM] InputMode UIOnly + cursor"));
	UKismetSystemLibrary::PrintString(World, TEXT("[GM] InputMode UIOnly + cursor"), true, true, FLinearColor::Green, 6.f);
}
