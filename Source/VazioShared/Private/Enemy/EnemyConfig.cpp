#include "Enemy/EnemyConfig.h"
#include "Enemy/EnemyTypes.h"

UEnemyConfig* UEnemyConfig::CreateDefaultConfig()
{
    UEnemyConfig* Config = NewObject<UEnemyConfig>();

    FEnemyArchetype NormalArchetype;
    NormalArchetype.BaseHP = 50.f;
    NormalArchetype.BaseDMG = 10.f;
    NormalArchetype.BaseSpeed = 300.f;
    NormalArchetype.BaseSize = 1.f;
    NormalArchetype.BaseXP = 1.f;
    NormalArchetype.Drop = EDropProfile::Normal;
    NormalArchetype.Death = EOnDeathBehavior::Normal;
    Config->Archetypes.Add(TEXT("NormalEnemy"), NormalArchetype);

    FEnemyArchetype HeavyArchetype;
    HeavyArchetype.BaseHP = 150.f;
    HeavyArchetype.BaseDMG = 20.f;
    HeavyArchetype.BaseSpeed = 150.f;
    HeavyArchetype.BaseSize = 1.2f;
    HeavyArchetype.BaseXP = 3.f;
    HeavyArchetype.Drop = EDropProfile::XPOnly;
    HeavyArchetype.Death = EOnDeathBehavior::Normal;
    Config->Archetypes.Add(TEXT("HeavyEnemy"), HeavyArchetype);

    FEnemyArchetype RangedArchetype;
    RangedArchetype.BaseHP = 40.f;
    RangedArchetype.BaseDMG = 15.f;
    RangedArchetype.BaseSpeed = 250.f;
    RangedArchetype.BaseSize = 1.f;
    RangedArchetype.BaseXP = 2.f;
    RangedArchetype.Drop = EDropProfile::XPOnly;
    RangedArchetype.Death = EOnDeathBehavior::Normal;
    Config->Archetypes.Add(TEXT("RangedEnemy"), RangedArchetype);

    FEnemyArchetype DashArchetype;
    DashArchetype.BaseHP = 60.f;
    DashArchetype.BaseDMG = 25.f;
    DashArchetype.BaseSpeed = 400.f;
    DashArchetype.BaseSize = 1.f;
    DashArchetype.BaseXP = 2.f;
    DashArchetype.Drop = EDropProfile::XPOnly;
    DashArchetype.Death = EOnDeathBehavior::Normal;
    DashArchetype.bCanDash = true;
    DashArchetype.DashCooldown = 3.f;
    DashArchetype.DashDistance = 500.f;
    Config->Archetypes.Add(TEXT("DashEnemy"), DashArchetype);

    FEnemyArchetype AuraArchetype;
    AuraArchetype.BaseHP = 80.f;
    AuraArchetype.BaseDMG = 12.f;
    AuraArchetype.BaseSpeed = 250.f;
    AuraArchetype.BaseSize = 1.1f;
    AuraArchetype.BaseXP = 3.f;
    AuraArchetype.Drop = EDropProfile::XPOnly;
    AuraArchetype.Death = EOnDeathBehavior::Normal;
    AuraArchetype.bHasAura = true;
    AuraArchetype.AuraRadius = 400.f;
    AuraArchetype.AuraDPS = 5.f;
    Config->Archetypes.Add(TEXT("AuraEnemy"), AuraArchetype);

    FEnemyArchetype SplitterArchetype;
    SplitterArchetype.BaseHP = 100.f;
    SplitterArchetype.BaseDMG = 15.f;
    SplitterArchetype.BaseSpeed = 200.f;
    SplitterArchetype.BaseSize = 1.3f;
    SplitterArchetype.BaseXP = 0.f;
    SplitterArchetype.Drop = EDropProfile::None;
    SplitterArchetype.Death = EOnDeathBehavior::Split;
    Config->Archetypes.Add(TEXT("SplitterSlime"), SplitterArchetype);

    FEnemyArchetype GoldArchetype;
    GoldArchetype.BaseHP = 30.f;
    GoldArchetype.BaseDMG = 8.f;
    GoldArchetype.BaseSpeed = 210.f;
    GoldArchetype.BaseSize = 1.f;
    GoldArchetype.BaseXP = 1.f;
    GoldArchetype.Drop = EDropProfile::Gold;
    GoldArchetype.Death = EOnDeathBehavior::Normal;
    Config->Archetypes.Add(TEXT("GoldEnemy"), GoldArchetype);

    FEnemyArchetype BurrowerBossArchetype;
    BurrowerBossArchetype.BaseHP = 2000.f;
    BurrowerBossArchetype.BaseDMG = 100.f;
    BurrowerBossArchetype.BaseSpeed = 200.f;
    BurrowerBossArchetype.BaseSize = 2.5f;
    BurrowerBossArchetype.BaseXP = 100.f;
    BurrowerBossArchetype.Drop = EDropProfile::Gold;
    BurrowerBossArchetype.Death = EOnDeathBehavior::Normal;
    Config->Archetypes.Add(TEXT("BurrowerBoss"), BurrowerBossArchetype);

    FEnemyArchetype VoidQueenBossArchetype;
    VoidQueenBossArchetype.BaseHP = 2500.f;
    VoidQueenBossArchetype.BaseDMG = 120.f;
    VoidQueenBossArchetype.BaseSpeed = 150.f;
    VoidQueenBossArchetype.BaseSize = 3.0f;
    VoidQueenBossArchetype.BaseXP = 150.f;
    VoidQueenBossArchetype.Drop = EDropProfile::Gold;
    VoidQueenBossArchetype.Death = EOnDeathBehavior::Normal;
    Config->Archetypes.Add(TEXT("VoidQueenBoss"), VoidQueenBossArchetype);

    FEnemyArchetype FallenWarlordBossArchetype;
    FallenWarlordBossArchetype.BaseHP = 3000.f;
    FallenWarlordBossArchetype.BaseDMG = 150.f;
    FallenWarlordBossArchetype.BaseSpeed = 180.f;
    FallenWarlordBossArchetype.BaseSize = 2.8f;
    FallenWarlordBossArchetype.BaseXP = 200.f;
    FallenWarlordBossArchetype.Drop = EDropProfile::Gold;
    FallenWarlordBossArchetype.Death = EOnDeathBehavior::Normal;
    Config->Archetypes.Add(TEXT("FallenWarlordBoss"), FallenWarlordBossArchetype);

    FEnemyArchetype HybridDemonBossArchetype;
    HybridDemonBossArchetype.BaseHP = 3500.f;
    HybridDemonBossArchetype.BaseDMG = 180.f;
    HybridDemonBossArchetype.BaseSpeed = 220.f;
    HybridDemonBossArchetype.BaseSize = 3.2f;
    HybridDemonBossArchetype.BaseXP = 250.f;
    HybridDemonBossArchetype.Drop = EDropProfile::Gold;
    HybridDemonBossArchetype.Death = EOnDeathBehavior::Normal;
    Config->Archetypes.Add(TEXT("HybridDemonBoss"), HybridDemonBossArchetype);

    return Config;
}
