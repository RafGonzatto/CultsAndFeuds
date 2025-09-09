#include "World/Common/Player/PlayerHealthComponent.h"

UPlayerHealthComponent::UPlayerHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
    OnDamaged.Broadcast(CurrentHealth);
}

void UPlayerHealthComponent::ReceiveDamage(float DamageAmount)
{
    if (DamageAmount <= 0.f || CurrentHealth <= 0.f) return;

    CurrentHealth = FMath::Max(0.f, CurrentHealth - DamageAmount);
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
    OnDamaged.Broadcast(CurrentHealth);

    if (CurrentHealth <= 0.f)
    {
        OnDeath.Broadcast();
    }
}

void UPlayerHealthComponent::SetMaxHealth(float NewMaxHealth)
{
    MaxHealth = FMath::Max(1.f, NewMaxHealth);
    CurrentHealth = FMath::Min(CurrentHealth, MaxHealth);
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
    OnDamaged.Broadcast(CurrentHealth);
}

void UPlayerHealthComponent::Heal(float Amount)
{
    if (Amount <= 0.f || CurrentHealth <= 0.f) return;
    CurrentHealth = FMath::Min(CurrentHealth + Amount, MaxHealth);
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
    OnDamaged.Broadcast(CurrentHealth);
}