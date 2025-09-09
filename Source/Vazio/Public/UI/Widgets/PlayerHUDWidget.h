#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UPlayerHealthComponent;
class UXPComponent;

UCLASS()
class VAZIO_API UPlayerHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Refer�ncias para os componentes do player que queremos monitorar
    UPROPERTY(BlueprintReadWrite, Category = "Components")
    TObjectPtr<UPlayerHealthComponent> HealthComponent;

    UPROPERTY(BlueprintReadWrite, Category = "Components")
    TObjectPtr<UXPComponent> XPComponent;

    // M�todos para atualizar a UI com base nos componentes
    UFUNCTION(BlueprintCallable, Category = "Updates")
    void UpdateHealthBar(float NewHealth);

    UFUNCTION(BlueprintCallable, Category = "Updates")
void UpdateXPBar(int32 CurrentXP, int32 XPToNextLevel);

    UFUNCTION(BlueprintCallable, Category = "Updates")
    void UpdateLevel(int32 NewLevel);

    // Fun��o para encontrar e vincular aos componentes
    UFUNCTION(BlueprintCallable, Category = "Setup")
    void BindToPlayerComponents();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // UI Elements - ser�o conectados pelo Blueprint
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UProgressBar* HealthBar;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UProgressBar* XPBar;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* LevelText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* HealthText;
};