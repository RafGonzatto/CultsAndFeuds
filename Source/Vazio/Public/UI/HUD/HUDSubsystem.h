#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HUDSubsystem.generated.h"

class SHUDRoot;

UCLASS()
class VAZIO_API UHUDSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // HUD Management
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ShowHUD();
    
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void HideHUD();
    
    UFUNCTION(BlueprintCallable, Category = "HUD")
    bool IsHUDVisible() const { return bIsHUDVisible; }

    // Update Methods (called by components)
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateHealth(float CurrentHealth, float MaxHealth);
    
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateXP(int32 CurrentXP, int32 XPToNextLevel);
    
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateLevel(int32 NewLevel);

private:
    void SetupDelegateBindings();
    void CleanupDelegateBindings();
    
    // UI Components
    TSharedPtr<SHUDRoot> HUDWidget;
    bool bIsHUDVisible = false;
};
