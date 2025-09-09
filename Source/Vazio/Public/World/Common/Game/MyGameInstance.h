#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

UCLASS()
class VAZIO_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UMyGameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;
	
	// Called when level completes loading on client
	virtual void LoadComplete(const float LoadTime, const FString& MapName) override;

private:
	// Initialize Steam login
	void InitializeSteamLogin();
	
	// Timer to check Steam status
	FTimerHandle SteamLoginTimer;
	void CheckSteamLoginStatus();
};
