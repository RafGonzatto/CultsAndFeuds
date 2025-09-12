#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyTypes.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEnemySpawn, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogEnemy, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogEconomy, Log, All);

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
struct VAZIO_API FEnemyInstanceModifiers
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bBig = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bImmovable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DissolveSeconds = 0.f;

    FEnemyInstanceModifiers()
    {
        bBig = false;
        bImmovable = false;
        DissolveSeconds = 0.f;
    }
};

USTRUCT(BlueprintType)
struct VAZIO_API FEnemyArchetype
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
struct VAZIO_API FTypeCount
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
struct VAZIO_API FCircleSpawn
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
struct VAZIO_API FSpawnEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSeconds = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FTypeCount> Linear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FCircleSpawn> Circles;

    FSpawnEvent()
    {
        TimeSeconds = 0.f;
    }
};
