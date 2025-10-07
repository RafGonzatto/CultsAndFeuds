#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BossTestPlayerController.generated.h"

class UBossAutoTestSubsystem;

UCLASS()
class VAZIO_API ABossTestPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ABossTestPlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

private:
    // Input handlers para teste dos bosses
    void OnTestKey1();  // Burrower
    void OnTestKey2();  // Void Queen
    void OnTestKey3();  // Fallen Warlord
    void OnTestKey4();  // Hybrid Demon
    void OnTestKey5();  // All Bosses
    void OnTestKey0();  // Stop Boss
    void OnTestKeyF1(); // Ativar/Desativar modo teste (F12 - renamed for legacy compatibility)

    UPROPERTY()
    TObjectPtr<UBossAutoTestSubsystem> BossTestSubsystem;

    void GetBossTestSubsystem();
};