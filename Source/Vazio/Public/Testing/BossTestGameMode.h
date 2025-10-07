#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BossTestGameMode.generated.h"

UCLASS()
class VAZIO_API ABossTestGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ABossTestGameMode();

protected:
    virtual void BeginPlay() override;
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

private:
    void SetupBossTestingEnvironment();
};