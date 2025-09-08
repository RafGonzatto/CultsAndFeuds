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
    // Tenta encontrar os componentes do player logo na constru��o
    BindToPlayerComponents();
}

void UPlayerHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    // Se n�o temos os componentes ainda, tenta vincul�-los
    if (!HealthComponent || !XPComponent)
    {
        BindToPlayerComponents();
    }
}

void UPlayerHUDWidget::BindToPlayerComponents()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!PlayerPawn)
    {
        UE_LOG(LogUI, Warning, TEXT("PlayerHUDWidget: PlayerPawn n�o encontrado"));
        return;
    }
    
    // Encontrar os componentes
    HealthComponent = PlayerPawn->FindComponentByClass<UPlayerHealthComponent>();
    XPComponent = PlayerPawn->FindComponentByClass<UXPComponent>();
    
    if (!HealthComponent)
    {
        UE_LOG(LogUI, Warning, TEXT("PlayerHUDWidget: HealthComponent n�o encontrado"));
    }
    else
    {
        // Registrar callbacks para updates
        HealthComponent->OnDamaged.AddDynamic(this, &UPlayerHUDWidget::UpdateHealthBar);
        // Atualiza��o inicial
        UpdateHealthBar(HealthComponent->CurrentHealth);
    }
    
    if (!XPComponent)
    {
        UE_LOG(LogUI, Warning, TEXT("PlayerHUDWidget: XPComponent n�o encontrado"));
    }
    else
    {
        // Registrar callbacks para updates
        XPComponent->OnXPChanged.AddDynamic(this, &UPlayerHUDWidget::UpdateXPBar);
        XPComponent->OnLevelChanged.AddDynamic(this, &UPlayerHUDWidget::UpdateLevel);
        // Atualiza��es iniciais
        UpdateXPBar(XPComponent->CurrentXP);
        UpdateLevel(XPComponent->CurrentLevel);
    }
}

void UPlayerHUDWidget::UpdateHealthBar(float NewHealth)
{
    if (!HealthComponent) return;
    
    const float HealthPercent = HealthComponent->GetHealthPercent();
    
    if (HealthBar)
    {
        HealthBar->SetPercent(HealthPercent);
    }
    
    if (HealthText)
    {
        const int32 CurrentHP = FMath::RoundToInt(HealthComponent->CurrentHealth);
        const int32 MaxHP = FMath::RoundToInt(HealthComponent->MaxHealth);
        HealthText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentHP, MaxHP)));
    }
    
    UE_LOG(LogUI, Verbose, TEXT("HUD: Health atualizado para %.1f/%.1f (%.1f%%)"), 
        HealthComponent->CurrentHealth, HealthComponent->MaxHealth, HealthPercent * 100.0f);
}

void UPlayerHUDWidget::UpdateXPBar(float NewXP)
{
    if (!XPComponent) return;
    
    const float XPPercent = XPComponent->GetXPPercent();
    
    if (XPBar)
    {
        XPBar->SetPercent(XPPercent);
    }
    
    UE_LOG(LogUI, Verbose, TEXT("HUD: XP atualizado para %.1f/%.1f (%.1f%%)"), 
        XPComponent->CurrentXP, XPComponent->XPToNextLevel, XPPercent * 100.0f);
}

void UPlayerHUDWidget::UpdateLevel(int32 NewLevel)
{
    if (!XPComponent) return;
    
    if (LevelText)
    {
        LevelText->SetText(FText::FromString(FString::Printf(TEXT("N�vel %d"), NewLevel)));
    }
    
    UE_LOG(LogUI, Log, TEXT("HUD: Level atualizado para %d"), NewLevel);
}

// Nota: O posicionamento (ficar centralizado na parte inferior) deve ser ajustado no Blueprint
// WBP_PlayerHUD ancorando o root em Bottom Center (Anchor Min/Max Y=1, X=0.5) e alinhamento 0.5
// Aqui apenas deixamos um log para lembrar caso não esteja correto.