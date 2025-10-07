#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/Console.h"
#include "BossAutoTestSubsystem.generated.h"

class UEnemySpawnerSubsystem;

UCLASS()
class VAZIO_API UBossAutoTestSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Console Commands - podem ser chamadas via console do editor
    UFUNCTION(Exec)
    void SpawnBurrower();

    UFUNCTION(Exec)
    void SpawnVoidQueen();

    UFUNCTION(Exec)
    void SpawnFallenWarlord();

    UFUNCTION(Exec)
    void SpawnHybridDemon();

    UFUNCTION(Exec)
    void SpawnAllBosses();

    UFUNCTION(Exec)
    void StopBoss();

    // Função para ativar modo de teste automático
    UFUNCTION(Exec)
    void StartAutoTest();

    UFUNCTION(Exec)
    void StopAutoTest();

    // Input bindings - teclas para ativar testes
    void SetupInputBindings();

    // Input handlers (públicas para acesso do PlayerController)
    void OnKey1Pressed();  // Burrower
    void OnKey2Pressed();  // Void Queen  
    void OnKey3Pressed();  // Fallen Warlord
    void OnKey4Pressed();  // Hybrid Demon
    void OnKey5Pressed();  // All Bosses
    void OnKey0Pressed();  // Stop

protected:

private:
    UPROPERTY()
    TObjectPtr<UEnemySpawnerSubsystem> EnemySpawner;

    bool bAutoTestMode = false;
    FTimerHandle AutoTestTimer;

    void GetEnemySpawner();
    void LogBossTest(const FString& Message);
};