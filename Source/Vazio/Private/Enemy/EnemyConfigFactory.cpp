#include "Enemy/EnemyConfig.h"
#include "Enemy/EnemyTypes.h"

// Static function to create default enemy config for testing
UEnemyConfig* UEnemyConfig::CreateDefaultConfig()
{
    UEnemyConfig* Config = NewObject<UEnemyConfig>();
    
    // NormalEnemy archetype
    FEnemyArchetype NormalArchetype;
    NormalArchetype.BaseHP = 50.f;
    NormalArchetype.BaseDMG = 10.f;
    NormalArchetype.BaseSpeed = 300.f;
    NormalArchetype.BaseSize = 1.f;
    NormalArchetype.BaseXP = 1.f;
    NormalArchetype.Drop = EDropProfile::Normal; // 5% chance for +1 Gold
    NormalArchetype.Death = EOnDeathBehavior::Normal;
    Config->Archetypes.Add(TEXT("NormalEnemy"), NormalArchetype);
    
    // HeavyEnemy archetype
    FEnemyArchetype HeavyArchetype;
    HeavyArchetype.BaseHP = 150.f;
    HeavyArchetype.BaseDMG = 20.f;
    HeavyArchetype.BaseSpeed = 150.f;
    HeavyArchetype.BaseSize = 1.2f;
    HeavyArchetype.BaseXP = 3.f;
    HeavyArchetype.Drop = EDropProfile::XPOnly;
    HeavyArchetype.Death = EOnDeathBehavior::Normal;
    Config->Archetypes.Add(TEXT("HeavyEnemy"), HeavyArchetype);
    
    // RangedEnemy archetype
    FEnemyArchetype RangedArchetype;
    RangedArchetype.BaseHP = 40.f;
    RangedArchetype.BaseDMG = 15.f;
    RangedArchetype.BaseSpeed = 250.f;
    RangedArchetype.BaseSize = 1.f;
    RangedArchetype.BaseXP = 2.f;
    RangedArchetype.Drop = EDropProfile::XPOnly;
    RangedArchetype.Death = EOnDeathBehavior::Normal;
    Config->Archetypes.Add(TEXT("RangedEnemy"), RangedArchetype);
    
    // DashEnemy archetype
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
    
    // AuraEnemy archetype
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
    
    // SplitterSlime archetype
    FEnemyArchetype SplitterArchetype;
    SplitterArchetype.BaseHP = 100.f;
    SplitterArchetype.BaseDMG = 15.f;
    SplitterArchetype.BaseSpeed = 200.f;
    SplitterArchetype.BaseSize = 1.3f;
    SplitterArchetype.BaseXP = 0.f; // Parents drop 0 XP
    SplitterArchetype.Drop = EDropProfile::None; // Parents drop no gold
    SplitterArchetype.Death = EOnDeathBehavior::Split;
    Config->Archetypes.Add(TEXT("SplitterSlime"), SplitterArchetype);
    
    // GoldEnemy archetype
    FEnemyArchetype GoldArchetype;
    GoldArchetype.BaseHP = 30.f;
    GoldArchetype.BaseDMG = 8.f;
    GoldArchetype.BaseSpeed = 210.f; // 70% of normal speed
    GoldArchetype.BaseSize = 1.f;
    GoldArchetype.BaseXP = 1.f;
    GoldArchetype.Drop = EDropProfile::Gold; // 100% chance for +10 Gold
    GoldArchetype.Death = EOnDeathBehavior::Normal;
    Config->Archetypes.Add(TEXT("GoldEnemy"), GoldArchetype);
    
    return Config;
}
