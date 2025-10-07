#pragma once

#include "CoreMinimal.h"
#include "Enemy/EnemyBase.h"
#include "Enemy/EnemyTypes.h"
#include "BossEnemy.generated.h"

class UEnemySpawnerSubsystem;
class ABossEnemy; // forward for delegate

USTRUCT(BlueprintType)
struct VAZIO_API FBossAttackPattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Pattern")
    FName PatternName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Pattern")
    float TelegraphTime = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Pattern")
    float Cooldown = 6.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Pattern")
    float ExecutionTime = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Pattern")
    bool bSummonsMinions = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Pattern", meta=(EditCondition="bSummonsMinions"))
    FName MinionType = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Pattern", meta=(EditCondition="bSummonsMinions"))
    int32 MinionCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Pattern", meta=(EditCondition="bSummonsMinions"))
    float MinionSpawnRadius = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Pattern", meta=(EditCondition="bSummonsMinions"))
    FEnemyInstanceModifiers MinionModifiers;

    FBossAttackPattern();
};

USTRUCT(BlueprintType)
struct VAZIO_API FBossPhaseDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Phase")
    float HealthThreshold = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Phase")
    float MovementSpeedMultiplier = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Phase")
    TArray<FBossAttackPattern> AttackPatterns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Phase")
    bool bEnableSummoningLoop = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Phase", meta=(EditCondition="bEnableSummoningLoop"))
    float SummonInterval = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Phase", meta=(EditCondition="bEnableSummoningLoop"))
    FName SummonType = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Phase", meta=(EditCondition="bEnableSummoningLoop"))
    int32 SummonCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Phase", meta=(EditCondition="bEnableSummoningLoop"))
    float SummonRadius = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Phase", meta=(EditCondition="bEnableSummoningLoop"))
    FEnemyInstanceModifiers SummonModifiers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Phase")
    float AttackIntervalOverride = 0.f;

    FBossPhaseDefinition();
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBossPhaseChanged, int32, NewPhaseIndex, const FBossPhaseDefinition&, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossHealthChanged, float, HealthFraction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossTelegraph, const FBossAttackPattern&, Pattern);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossAttackExecuted, const FBossAttackPattern&, Pattern);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossDefeated, ABossEnemy*, Boss);

UCLASS(BlueprintType, Blueprintable)
class VAZIO_API ABossEnemy : public AEnemyBase
{
    GENERATED_BODY()
public:
    ABossEnemy();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void HandleDeath(bool bIsParentParam = false) override;
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Boss")
    float GetHealthFraction() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Boss")
    int32 GetCurrentPhaseIndex() const { return CurrentPhaseIndex; }

    // Blueprint-safe accessor (cannot expose raw pointer to struct). Returns a copy.
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Boss")
    FBossPhaseDefinition GetCurrentPhase() const;

    UPROPERTY(BlueprintAssignable, Category="Boss|Events")
    FOnBossPhaseChanged OnBossPhaseChanged;
    UPROPERTY(BlueprintAssignable, Category="Boss|Events")
    FOnBossHealthChanged OnBossHealthChanged;
    UPROPERTY(BlueprintAssignable, Category="Boss|Events")
    FOnBossTelegraph OnBossTelegraph;
    UPROPERTY(BlueprintAssignable, Category="Boss|Events")
    FOnBossAttackExecuted OnBossAttackExecuted;
    UPROPERTY(BlueprintAssignable, Category="Boss|Events")
    FOnBossDefeated OnBossDefeated;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Config")
    FText BossDisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Config")
    TArray<FBossPhaseDefinition> Phases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Config")
    float DefaultAttackInterval = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Config")
    bool bLoopPhasePatterns = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Movement")
    float MovementRadius = 900.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Movement")
    float MovementHeightOffset = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Movement")
    float MovementInterpSpeed = 1.75f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Movement")
    bool bFacePlayer = true;

    virtual void HandlePhaseStarted(const FBossPhaseDefinition& Phase);
    virtual void PerformAttackPattern(const FBossAttackPattern& Pattern);
    virtual void PerformMovementPattern(float DeltaTime);

    void EvaluatePhase();
    void EnterPhase(int32 NewPhaseIndex);
    void TickAttackCycle(float DeltaTime);
    void StartTelegraph(const FBossAttackPattern& Pattern);
    void FinishTelegraph();
    void ExecuteAttackPattern(const FBossAttackPattern& Pattern);
    void HandleSummoning(float DeltaTime);
    void SpawnSummonedMinions(FName MinionType, int32 Count, float Radius, const FEnemyInstanceModifiers& Mods);
    void CacheSpawner();

    // Internal raw pointer style access (not exposed to BP)
    const FBossPhaseDefinition* GetCurrentPhasePtr() const { return Phases.IsValidIndex(CurrentPhaseIndex) ? &Phases[CurrentPhaseIndex] : nullptr; }

    int32 CurrentPhaseIndex = INDEX_NONE;
    int32 CurrentPatternIndex = 0;
    float AttackTimer = 0.f;
    float SummonTimer = 0.f;
    bool bTelegraphActive = false;
    bool bHasCurrentPattern = false;
    float CachedMaxHP = 0.f;

    FBossAttackPattern CurrentPattern;
    TWeakObjectPtr<UEnemySpawnerSubsystem> CachedSpawnerSubsystem;

    // Para logs de debug de posição
    FVector LastLoggedPosition = FVector::ZeroVector;
    float LastPositionLogTime = 0.f;
};
