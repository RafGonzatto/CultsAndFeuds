#include "World/Common/Player/XPComponent.h"

UXPComponent::UXPComponent() { PrimaryComponentTick.bCanEverTick = false; }

void UXPComponent::AddXP(float Amount) {
    if (Amount <= 0.f) return;
    CurrentXP += Amount;
    UE_LOG(LogTemp, Log, TEXT("[XP] +%f → %f/%f"), Amount, CurrentXP, XPToNextLevel);
    OnXPChanged.Broadcast(CurrentXP);
    while (CurrentXP >= XPToNextLevel) {
        CurrentXP -= XPToNextLevel;
        ++CurrentLevel;
        UE_LOG(LogTemp, Log, TEXT("[XP] Level up! → %d"), CurrentLevel);
        OnLevelChanged.Broadcast(CurrentLevel);
        // opcional: XPToNextLevel *= 1.1f;
    }
}
float UXPComponent::GetXPPercent() const { return XPToNextLevel > 0.f ? CurrentXP / XPToNextLevel : 0.f; }
