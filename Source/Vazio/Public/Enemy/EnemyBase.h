#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"  
#include "Enemy/EnemyTypes.h"
#include "EnemyBase.generated.h"

class UEnemyDropComponent;
class UEnemyAuraComponent;
class UHealthComponent;
class UPointLightComponent;

UCLASS(BlueprintType, Blueprintable)
class VAZIO_API AEnemyBase : public ACharacter
{
    GENERATED_BODY()

public:
    AEnemyBase();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Enemy")
    virtual void HandleDeath(bool bIsParentParam = false);

    UFUNCTION(BlueprintCallable, Category = "Enemy")
    void ApplyArchetypeAndModifiers(const FEnemyArchetype& Arch, const FEnemyInstanceModifiers& Mods);

    UFUNCTION(BlueprintCallable, Category = "Enemy")
    void StartDissolveTimer();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    bool bIsParent = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UEnemyDropComponent> DropComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UEnemyAuraComponent> AuraComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<class UStaticMeshComponent> VisualMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<class UPointLightComponent> DebugLight;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    FEnemyArchetype CurrentArchetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    FEnemyInstanceModifiers CurrentModifiers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    float CurrentHP = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    float MaxHP = 100.f;

    FTimerHandle DissolveTimerHandle;
    FTimerHandle DashCooldownHandle;

    UFUNCTION()
    void OnDissolveComplete();

    UFUNCTION(BlueprintImplementableEvent, Category = "Enemy")
    void OnApplyVisualEffects();

    UFUNCTION(BlueprintImplementableEvent, Category = "Enemy")
    void OnStartDissolve();

    virtual void SetupMovement();
    virtual void HandleDashLogic(float DeltaTime);

public:
    FORCEINLINE float GetCurrentHP() const { return CurrentHP; }
    FORCEINLINE float GetMaxHP() const { return MaxHP; }
    FORCEINLINE const FEnemyArchetype& GetArchetype() const { return CurrentArchetype; }
    FORCEINLINE const FEnemyInstanceModifiers& GetModifiers() const { return CurrentModifiers; }

    UFUNCTION(BlueprintCallable, Category = "Enemy")
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(BlueprintCallable, Category = "Enemy")
    void TakeDamageSimple(float Damage);

    // Performance optimization function
    void PerformanceOptimization();

    // AI movement
    void ChasePlayer();

    // Toggle base chase behavior (ranged enemies disable this)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI")
    bool bUseBaseChase = true;

    // Damage system
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    
    float LastDamageTime = 0.0f; // Damage cooldown
    bool bFirstTickLogged = false; // Para diagnosticar se Tick roda pelo menos uma vez

    // MOVEMENT LOGGING STATE
    // Armazena a posição do frame anterior para calcular delta de movimento
    FVector PreviousLocation = FVector::ZeroVector;
    // Última posição do alvo (player) usada no cálculo de perseguição
    FVector LastTargetLocation = FVector::ZeroVector;
    // Se já inicializou PreviousLocation
    bool bHasPreviousLocation = false;
};
