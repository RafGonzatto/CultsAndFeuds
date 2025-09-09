#include "UI/Widgets/PlayerHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "World/Common/Player/PlayerHealthComponent.h"
#include "World/Common/Player/XPComponent.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogUI, Log, All);

void UPlayerHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Log inicial para ajudar debug de criação duplicada
    UE_LOG(LogUI, Log, TEXT("HUD: NativeConstruct iniciado (Addr=%p)"), this);
    // Tenta encontrar os componentes do player logo na construção
    BindToPlayerComponents();
}

void UPlayerHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    // Se não temos os componentes ainda, tenta vinculá-los
    if (!HealthComponent || !XPComponent)
    {
        BindToPlayerComponents();
    }
}

void UPlayerHUDWidget::BindToPlayerComponents()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!PlayerPawn) return;

    // Encontrar os componentes
    HealthComponent = PlayerPawn->FindComponentByClass<UPlayerHealthComponent>();
    XPComponent = PlayerPawn->FindComponentByClass<UXPComponent>();

    if (HealthComponent)
    {
        // Registrar callbacks para updates
        HealthComponent->OnDamaged.AddDynamic(this, &UPlayerHUDWidget::UpdateHealthBar);
        // Atualização inicial
        UpdateHealthBar(HealthComponent->GetCurrentHealth());
    }
    if (XPComponent)
    {
        // Registrar callbacks para updates
        XPComponent->OnXPChanged.AddDynamic(this, &UPlayerHUDWidget::UpdateXPBar);
        XPComponent->OnLevelChanged.AddDynamic(this, &UPlayerHUDWidget::UpdateLevel);
        // Atualizações iniciais - corrigido para passar ambos os parâmetros
        UpdateXPBar(XPComponent->GetCurrentXP(), XPComponent->GetXPToNextLevel());
        UpdateLevel(XPComponent->GetCurrentLevel());
    }
}

void UPlayerHUDWidget::UpdateHealthBar(float NewHealth)
{
    if (!HealthComponent) return;
    
    const float HealthPercent = HealthComponent->GetHealthPercent();
    
    if (HealthBar) HealthBar->SetPercent(HealthPercent);
    if (HealthText)
    {
        const int32 CurrentHP = FMath::RoundToInt(HealthComponent->GetCurrentHealth());
        const int32 MaxHP = FMath::RoundToInt(HealthComponent->GetMaxHealth());
        HealthText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentHP, MaxHP)));
    }
    
    UE_LOG(LogUI, Verbose, TEXT("HUD: Health atualizado: %.1f/%.1f (%.1f%%)"), HealthComponent->GetCurrentHealth(), HealthComponent->GetMaxHealth(), HealthPercent * 100.f);
}

void UPlayerHUDWidget::UpdateXPBar(int32 CurrentXP, int32 XPToNextLevel)
{
    if (!XPBar) return;
    
    float Percentage = XPToNextLevel > 0 ? (float)CurrentXP / (float)XPToNextLevel : 0.0f;
    XPBar->SetPercent(Percentage);
    
    // NOTE: Since there's no XPTextBlock declared in the header, we'll skip updating XP text
    // If you need XP text, add a UTextBlock* XPText property to the header file with meta = (BindWidget)
}

void UPlayerHUDWidget::UpdateLevel(int32 NewLevel)
{
    if (LevelText)
    {
        LevelText->SetText(FText::FromString(FString::Printf(TEXT("Nível %d"), NewLevel)));
    }
    
    UE_LOG(LogUI, Log, TEXT("HUD: Level atualizado para %d"), NewLevel);
}

// Nota: O posicionamento (ficar centralizado na parte inferior) deve ser ajustado no Blueprint
// WBP_PlayerHUD ancorando o root em Bottom Center (Anchor Min/Max Y=1, X=0.5) e alinhamento 0.5
// Aqui apenas deixamos um log para lembrar caso não esteja correto.