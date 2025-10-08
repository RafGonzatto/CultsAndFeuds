#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyTypes.generated.h"

VAZIOSHARED_API DECLARE_LOG_CATEGORY_EXTERN(LogEnemySpawn, Log, All);
VAZIOSHARED_API DECLARE_LOG_CATEGORY_EXTERN(LogEnemy, Log, All);
VAZIOSHARED_API DECLARE_LOG_CATEGORY_EXTERN(LogEconomy, Log, All);
VAZIOSHARED_API DECLARE_LOG_CATEGORY_EXTERN(LogBoss, Log, All);

class USoundBase;

USTRUCT(BlueprintType)
struct VAZIOSHARED_API FBaseRewards
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 XPValue = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GoldValue = 0;

    FBaseRewards()
    {
        XPValue = 1;
        GoldValue = 0;
    }
};

UENUM(BlueprintType)
enum class EDropProfile : uint8
{
    None,
    Normal,
    Gold,
    XPOnly
};

UENUM(BlueprintType)
enum class EOnDeathBehavior : uint8
{
    Normal,
    Split,
    Explode
};

USTRUCT(BlueprintType)
struct VAZIOSHARED_API FEnemyInstanceModifiers
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bBig = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bImmovable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DissolveSeconds = 0.f;

    // Multiplier properties for JSON parsing support
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HealthMultiplier = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DamageMultiplier = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpeedMultiplier = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ScaleMultiplier = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RewardMultiplier = 1.f;

    FEnemyInstanceModifiers()
    {
        bBig = false;
        bImmovable = false;
        DissolveSeconds = 0.f;
        HealthMultiplier = 1.f;
        DamageMultiplier = 1.f;
        SpeedMultiplier = 1.f;
        ScaleMultiplier = 1.f;
        RewardMultiplier = 1.f;
    }
};

USTRUCT(BlueprintType)
struct VAZIOSHARED_API FEnemyArchetype
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseHP = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseDMG = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseSpeed = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseSize = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseXP = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDropProfile Drop = EDropProfile::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EOnDeathBehavior Death = EOnDeathBehavior::Normal;

    // Dash properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanDash = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DashCooldown = 3.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DashDistance = 500.f;

    // Aura properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasAura = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AuraRadius = 400.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AuraDPS = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBaseRewards BaseRewards;

    FEnemyArchetype()
    {
        BaseHP = 100.f;
        BaseDMG = 10.f;
        BaseSpeed = 300.f;
        BaseSize = 1.f;
        BaseXP = 1.f;
        Drop = EDropProfile::None;
        Death = EOnDeathBehavior::Normal;
        bCanDash = false;
        DashCooldown = 3.f;
        DashDistance = 500.f;
        bHasAura = false;
        AuraRadius = 400.f;
        AuraDPS = 5.f;
    }
};

USTRUCT(BlueprintType)
struct VAZIOSHARED_API FTypeCount
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FEnemyInstanceModifiers Mods;

    FTypeCount()
    {
        Type = NAME_None;
        Count = 1;
    }
};

USTRUCT(BlueprintType)
struct VAZIOSHARED_API FCircleSpawn
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FEnemyInstanceModifiers Mods;

    FCircleSpawn()
    {
        Type = NAME_None;
        Count = 1;
        Radius = 1000.f;
    }
};

USTRUCT(BlueprintType)
struct VAZIOSHARED_API FSpawnEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSeconds = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FTypeCount> Linear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FCircleSpawn> Circles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllowDuringBossEncounter = true;

    // Additional members for JSON parsing support
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EnemyType = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnRadius = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FEnemyInstanceModifiers Modifiers;

    FSpawnEvent()
    {
        TimeSeconds = 0.f;
        bAllowDuringBossEncounter = true;
        EnemyType = NAME_None;
        Count = 1;
        SpawnRadius = 500.f;
    }
};

USTRUCT(BlueprintType)
struct VAZIOSHARED_API FBossSpawnEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName BossType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WarningLeadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Announcement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bFinalEncounter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPauseRegularSpawns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ResumeDelay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EntranceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSpawnOutOfView;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundBase* WarningSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FEnemyInstanceModifiers BossModifiers;

    // Additional members for JSON parsing support
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TriggerTime = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WarningDuration = 3.f;

    FBossSpawnEntry()
    {
        TimeSeconds = 0.f;
        BossType = NAME_None;
        WarningLeadTime = 5.f;
        Announcement = FText::GetEmpty();
        bFinalEncounter = false;
        bPauseRegularSpawns = true;
        ResumeDelay = 5.f;
        EntranceDistance = 1500.f;
        bSpawnOutOfView = false;
        WarningSound = nullptr;
        TriggerTime = 0.f;
        WarningDuration = 3.f;
    }
};
