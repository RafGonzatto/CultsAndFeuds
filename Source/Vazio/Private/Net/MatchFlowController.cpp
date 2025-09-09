#include "Net/MatchFlowController.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"

UMatchFlowController::UMatchFlowController()
	: LoadingScreenWidget(nullptr)
	, CurrentState(EMatchFlowState::Idle)
{
}

void UMatchFlowController::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	// Get session subsystem
	SessionSubsystem = GetGameInstance()->GetSubsystem<USessionSubsystem>();
	
	if (SessionSubsystem)
	{
		// Bind to session events
		SessionSubsystem->OnCreateSessionCompleteEvent.AddDynamic(this, &UMatchFlowController::OnCreateSessionComplete);
		SessionSubsystem->OnJoinSessionCompleteEvent.AddDynamic(this, &UMatchFlowController::OnJoinSessionComplete);
		SessionSubsystem->OnDestroySessionCompleteEvent.AddDynamic(this, &UMatchFlowController::OnDestroySessionComplete);
		
		UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] Initialized and bound to SessionSubsystem"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MatchFlowController] Failed to get SessionSubsystem"));
	}
}

void UMatchFlowController::Deinitialize()
{
	// Clean up loading screen
	DestroyLoadingScreenWidget();
	
	// Unbind from session events
	if (SessionSubsystem)
	{
		SessionSubsystem->OnCreateSessionCompleteEvent.RemoveDynamic(this, &UMatchFlowController::OnCreateSessionComplete);
		SessionSubsystem->OnJoinSessionCompleteEvent.RemoveDynamic(this, &UMatchFlowController::OnJoinSessionComplete);
		SessionSubsystem->OnDestroySessionCompleteEvent.RemoveDynamic(this, &UMatchFlowController::OnDestroySessionComplete);
	}
	
	Super::Deinitialize();
}

void UMatchFlowController::StartHosting(const FString& MapName, const FString& Difficulty)
{
	if (CurrentState != EMatchFlowState::Idle)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] Cannot start hosting - not in Idle state (current: %d)"), (int32)CurrentState);
		return;
	}
	
	// Try to get SessionSubsystem if not already cached
	if (!SessionSubsystem && GetGameInstance())
	{
		SessionSubsystem = GetGameInstance()->GetSubsystem<USessionSubsystem>();
		if (SessionSubsystem)
		{
			// Bind events if we just got the subsystem
			SessionSubsystem->OnCreateSessionCompleteEvent.AddDynamic(this, &UMatchFlowController::OnCreateSessionComplete);
			SessionSubsystem->OnJoinSessionCompleteEvent.AddDynamic(this, &UMatchFlowController::OnJoinSessionComplete);
			SessionSubsystem->OnDestroySessionCompleteEvent.AddDynamic(this, &UMatchFlowController::OnDestroySessionComplete);
			UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] SessionSubsystem bound successfully"));
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] Starting to host session: Map=%s, Difficulty=%s"), *MapName, *Difficulty);
	
	if (!SessionSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] SessionSubsystem not available - starting single player session"));
		StartSinglePlayerSession(MapName, Difficulty);
		return;
	}
	
	SetState(EMatchFlowState::Hosting);
	ShowLoadingScreen(TEXT("Creating session..."));
	
	SessionSubsystem->HostArenaSession(MapName, Difficulty);
}

void UMatchFlowController::StartJoining(int32 SessionIndex)
{
	if (CurrentState != EMatchFlowState::Idle)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] Cannot start joining - not in Idle state (current: %d)"), (int32)CurrentState);
		return;
	}
	
	if (!SessionSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[MatchFlowController] Cannot start joining - SessionSubsystem is null"));
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] Starting to join session at index: %d"), SessionIndex);
	
	SetState(EMatchFlowState::Joining);
	ShowLoadingScreen(TEXT("Joining session..."));
	
	SessionSubsystem->JoinSessionByIndex(SessionIndex);
}

void UMatchFlowController::LeaveMatch()
{
	if (!IsInMatch())
	{
		UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] Not in match - nothing to leave"));
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] Leaving match"));
	ShowLoadingScreen(TEXT("Leaving match..."));
	
	if (SessionSubsystem)
	{
		SessionSubsystem->DestroySession();
	}
	
	// Return to city
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("City_Main"));
	SetState(EMatchFlowState::Idle);
}

bool UMatchFlowController::IsInMatch() const
{
	return CurrentState == EMatchFlowState::InMatchHost || CurrentState == EMatchFlowState::InMatchClient;
}

void UMatchFlowController::ShowLoadingScreen(const FString& LoadingText)
{
	CreateLoadingScreenWidget();
	
	if (LoadingScreenWidget)
	{
		LoadingScreenWidget->SetLoadingText(LoadingText);
		LoadingScreenWidget->ShowLoadingScreen();
	}
}

void UMatchFlowController::HideLoadingScreen()
{
	if (LoadingScreenWidget)
	{
		LoadingScreenWidget->HideLoadingScreen();
	}
}

void UMatchFlowController::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] OnCreateSessionComplete: %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));
	
	if (bWasSuccessful)
	{
		SetState(EMatchFlowState::LoadingHost);
		ShowLoadingScreen(TEXT("Loading arena..."));
		
		// Start the session first
		if (SessionSubsystem)
		{
			SessionSubsystem->StartSession();
		}
		
		// As host, we need to travel to the Battle_Main level
		FString TravelURL = TEXT("Battle_Main");
		
		// Add selected map and difficulty as URL options if available
		bool bFirstParam = true;
		if (SessionSubsystem)
		{
			FString MapName = SessionSubsystem->GetCurrentMapName();
			FString Difficulty = SessionSubsystem->GetCurrentDifficulty();
			
			if (!MapName.IsEmpty())
			{
				TravelURL += bFirstParam ? TEXT("?") : TEXT("&");
				TravelURL += TEXT("SelectedMap=") + MapName;
				bFirstParam = false;
			}
			if (!Difficulty.IsEmpty())
			{
				TravelURL += bFirstParam ? TEXT("?") : TEXT("&");
				TravelURL += TEXT("Difficulty=") + Difficulty;
				bFirstParam = false;
			}
		}
		
		TravelURL += bFirstParam ? TEXT("?listen") : TEXT("&listen");
		
		// Force BattleGameMode for Battle_Main level
		TravelURL += TEXT("&game=/Script/Vazio.BattleGameMode");
		
		UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] Host traveling to: %s"), *TravelURL);
		
		// Server travel to Battle_Main as listen server
		if (UWorld* World = GetWorld())
		{
			World->ServerTravel(TravelURL);
		}
		
		SetState(EMatchFlowState::InMatchHost);
		
		// Start timer to check if anyone joins (30 seconds)
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(SoloPlayTimerHandle, this, &UMatchFlowController::CheckForSoloPlay, 30.0f, false);
			UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] Solo play timer started - will check in 30 seconds"));
		}
	}
	else
	{
		SetState(EMatchFlowState::Idle);
		HideLoadingScreen();
		
		// Show error message
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Failed to create session"));
		}
	}
}

void UMatchFlowController::OnJoinSessionComplete(FName SessionName, int32 Result)
{
	UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] OnJoinSessionComplete: %d"), (int32)Result);
	
	if (Result == 0) // EOnJoinSessionCompleteResult::Success
	{
		SetState(EMatchFlowState::LoadingClient);
		ShowLoadingScreen(TEXT("Connecting to arena..."));
		
		// Session joining will handle client travel via SessionSubsystem
	}
	else
	{
		SetState(EMatchFlowState::Idle);
		HideLoadingScreen();
		
		// Show error message
		FString ErrorMsg = TEXT("Failed to join session");
		switch (Result)
		{
		case EOnJoinSessionCompleteResult::SessionIsFull:
			ErrorMsg = TEXT("Session is full");
			break;
		case EOnJoinSessionCompleteResult::SessionDoesNotExist:
			ErrorMsg = TEXT("Session no longer exists");
			break;
		case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
			ErrorMsg = TEXT("Could not retrieve session address");
			break;
		default:
			break;
		}
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, *ErrorMsg);
		}
	}
}

void UMatchFlowController::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] OnDestroySessionComplete: %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));
	
	SetState(EMatchFlowState::Idle);
	HideLoadingScreen();
}

void UMatchFlowController::SetState(EMatchFlowState NewState)
{
	if (CurrentState != NewState)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] State change: %d -> %d"), (int32)CurrentState, (int32)NewState);
		CurrentState = NewState;
	}
}

void UMatchFlowController::CreateLoadingScreenWidget()
{
	if (!LoadingScreenWidget)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			LoadingScreenWidget = CreateWidget<ULoadingScreenWidget>(PC, ULoadingScreenWidget::StaticClass());
			if (LoadingScreenWidget)
			{
				LoadingScreenWidget->AddToViewport(1000); // High Z-order to be on top
				UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] Created loading screen widget"));
			}
		}
	}
}

void UMatchFlowController::DestroyLoadingScreenWidget()
{
	if (LoadingScreenWidget)
	{
		LoadingScreenWidget->RemoveFromParent();
		LoadingScreenWidget = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] Destroyed loading screen widget"));
	}
}

void UMatchFlowController::StartSinglePlayerSession(const FString& MapName, const FString& Difficulty)
{
	UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] Starting single player session: Map=%s, Difficulty=%s"), *MapName, *Difficulty);
	
	SetState(EMatchFlowState::LoadingHost);
	ShowLoadingScreen(TEXT("Loading single player arena..."));
	
	// Build travel URL for single player
	FString TravelURL = TEXT("Battle_Main");
	bool bFirstParam = true;
	
	if (!MapName.IsEmpty())
	{
		TravelURL += bFirstParam ? TEXT("?") : TEXT("&");
		TravelURL += TEXT("SelectedMap=") + MapName;
		bFirstParam = false;
	}
	if (!Difficulty.IsEmpty())
	{
		TravelURL += bFirstParam ? TEXT("?") : TEXT("&");
		TravelURL += TEXT("Difficulty=") + Difficulty;
		bFirstParam = false;
	}
	
	// Add single player flag
	TravelURL += bFirstParam ? TEXT("?SinglePlayer=true") : TEXT("&SinglePlayer=true");
	
	// Force BattleGameMode for Battle_Main level
	TravelURL += TEXT("&game=/Script/Vazio.BattleGameMode");
	
	UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] Single player traveling to: %s"), *TravelURL);
	
	// Use the full URL with GameMode parameter for single player
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::OpenLevel(World, *TravelURL);
	}
	
	SetState(EMatchFlowState::InMatchHost);
}

void UMatchFlowController::CheckForSoloPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] Checking for solo play..."));
	
	if (UWorld* World = GetWorld())
	{
		// Count connected players
		int32 PlayerCount = 0;
		for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			if (APlayerController* PC = Iterator->Get())
			{
				if (PC->IsLocalPlayerController() || PC->GetNetConnection())
				{
					PlayerCount++;
				}
			}
		}
		
		UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] Current player count: %d"), PlayerCount);
		
		if (PlayerCount <= 1)
		{
			// Only host is connected, continue as single player
			UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] No other players joined - continuing as single player"));
			
			// Show message to player
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("No other players joined - playing solo!"));
			}
			
			// We're already in the level, just continue playing
			HideLoadingScreen();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[MatchFlowController] Other players have joined - multiplayer session active"));
			HideLoadingScreen();
		}
	}
	
	// Clear the timer handle
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SoloPlayTimerHandle);
	}
}
