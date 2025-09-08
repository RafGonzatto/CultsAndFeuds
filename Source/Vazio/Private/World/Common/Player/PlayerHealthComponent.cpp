#include "World/Common/Player/PlayerHealthComponent.h"
#include "GameFramework/Actor.h"

UPlayerHealthComponent::UPlayerHealthComponent() {
    PrimaryComponentTick.bCanEverTick = false;
    CurrentHealth = MaxHealth;
}

void UPlayerHealthComponent::BeginPlay() {
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
}

void UPlayerHealthComponent::ReceiveDamage(float DamageAmount) {
    if (DamageAmount <= 0.f || CurrentHealth <= 0.f) return;
    CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.f, MaxHealth);
    UE_LOG(LogTemp, Log, TEXT("[PlayerHealth] %s damaged by %f → HP=%f"), *GetOwner()->GetName(), DamageAmount, CurrentHealth);
    OnDamaged.Broadcast(CurrentHealth);
    if (CurrentHealth <= 0.f) {
        UE_LOG(LogTemp, Warning, TEXT("[PlayerHealth] %s died"), *GetOwner()->GetName());
        OnDeath.Broadcast();
    }
}

float UPlayerHealthComponent::GetHealthPercent() const {
    return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f;
}
