#include "World/Common/GameModes/MainMenuGameMode.h"

#include "UI/Widgets/MainMenuWidget.h"
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
	// Evita erro de SpawnActor e mantém um Pawn simples
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

	// Chama SpawnMenu no próximo tick para garantir viewport pronto
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

	// Configurar visibilidade ANTES de adicionar à viewport
	Menu->SetVisibility(ESlateVisibility::Visible);

	// USAR Z-ORDER MÁXIMO POSSÍVEL
	Menu->AddToViewport(32767);  // Z-order máximo (int16 max)
	
	UE_LOG(LogTemp, Warning, TEXT("[GM] AddToViewport com Z-order 32767"));
	UKismetSystemLibrary::PrintString(World, TEXT("[GM] Widget adicionado com Z-order MÁXIMO"), true, true, FLinearColor::Green, 6.f);

	// FORÇAR O WIDGET PARA O TOPO DE TODAS AS CAMADAS
	if (UGameViewportClient* ViewportClient = World->GetGameViewport())
	{
		// Remove e re-adiciona para garantir que fique no topo
		Menu->RemoveFromParent();  // API atualizada
		Menu->AddToViewport(32767);
		
		UE_LOG(LogTemp, Warning, TEXT("[GM] Widget removido e re-adicionado para forçar topo"));
	}

	// Configurar Input Mode para UI
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = true;

	// ADICIONAR MENSAGEM DE DEBUG GIGANTE NA TELA
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Red, TEXT("*** WIDGET DEVERIA ESTAR VISÍVEL AGORA ***"));
		GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Yellow, TEXT("Se você vê esta mensagem, o sistema está funcionando"));
		GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Green, TEXT("O widget vermelho deveria cobrir toda a tela"));
	}

	UE_LOG(LogTemp, Warning, TEXT("[GM] Menu configurado completamente com debug messages"));
}
