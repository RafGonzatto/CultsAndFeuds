#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Net/SessionSubsystem.h"
#include "Net/LoadingScreenWidget.h"
#include "MatchFlowController.generated.h"

UENUM(BlueprintType)
enum class EMatchFlowState : uint8
{
	Idle,
	Hosting,
	LoadingHost,
	InMatchHost,
	Joining,
	LoadingClient,
	InMatchClient
};

UCLASS()
class VAZIO_API UMatchFlowController : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMatchFlowController();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Match Flow Control
	UFUNCTION(BlueprintCallable)
	void StartHosting(const FString& MapName, const FString& Difficulty);
	
	UFUNCTION(BlueprintCallable)
	void StartJoining(int32 SessionIndex);
	
	UFUNCTION(BlueprintCallable)
	void LeaveMatch();
	
	// Single Player fallback
	void StartSinglePlayerSession(const FString& MapName, const FString& Difficulty);
	
	UFUNCTION(BlueprintCallable)
	EMatchFlowState GetCurrentState() const { return CurrentState; }
	
	UFUNCTION(BlueprintCallable)
	bool IsInMatch() const;

	// Loading Screen Management
	UFUNCTION(BlueprintCallable)
	void ShowLoadingScreen(const FString& LoadingText);
	
	UFUNCTION(BlueprintCallable)
	void HideLoadingScreen();

private:
	UPROPERTY()
	USessionSubsystem* SessionSubsystem;
	
	UPROPERTY()
	ULoadingScreenWidget* LoadingScreenWidget;
	
	EMatchFlowState CurrentState;
	
	// Session callbacks
	UFUNCTION()
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	
	UFUNCTION()
	void OnJoinSessionComplete(FName SessionName, int32 Result);
	
	UFUNCTION()
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	// State management
	void SetState(EMatchFlowState NewState);
	
	// Loading screen management
	void CreateLoadingScreenWidget();
	
	// Solo play timer
	FTimerHandle SoloPlayTimerHandle;
	void CheckForSoloPlay();
	void DestroyLoadingScreenWidget();
};
