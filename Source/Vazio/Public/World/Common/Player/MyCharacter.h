#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "World/Common/Player/PlayerHealthComponent.h"
#include "World/Common/Player/XPComponent.h"
#include "Gameplay/Upgrades/UpgradeSystem.h"
#include "MyCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;
class USkeletalMesh;
class UAnimInstance;
class USphereComponent;
class SLevelUpModal;

UCLASS()
class VAZIO_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMyCharacter();

public:
	// Override TakeDamage to route damage to PlayerHealthComponent
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Assets de configuraÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â§ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â£o
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom Character")
	USkeletalMesh* CustomSkeletalMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom Character")
	TSubclassOf<UAnimInstance> CustomAnimInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom Character")
	USkeletalMesh* WeaponMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom Character")
	UAnimMontage* AttackMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom Character")
	float AttackDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom Character")
	float AttackRange = 200.0f;

protected:
	// Camera components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComponent;

	// Health and XP components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats")
	UPlayerHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats")
	UXPComponent* XPComponent;

	// Damage detection sphere
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	USphereComponent* DamageSphere;

	// Weapon system removed - was part of old Swarm system
	// TODO: Implement new generic weapon system if needed

private:
	void SpawnDefaultWeapons();

public:
	// Input handling
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Weapon management - TODO: Implement new weapon system
	bool IsInSwarmBattleLevel() const;

	// Level Up System
	UFUNCTION()
	void OnPlayerLevelUp(int32 NewLevel);
	
	void ShowLevelUpModal(const TArray<FUpgradeData>& Upgrades);
	void OnUpgradeChosen(EUpgradeType ChosenType);
	void CloseLevelUpModal();

	// Movement input functions
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	// Combat functions
	void Attack();
	void PerformAttack();

	// Utility functions
	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	float GetCurrentXP() const;

	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	int32 GetCurrentLevel() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TakeDamageFromEnemy(float Damage);

	// Overlap functions for damage detection
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Getters for components
	FORCEINLINE UPlayerHealthComponent* GetHealthComponent() const { return HealthComponent; }
	FORCEINLINE UXPComponent* GetXPComponent() const { return XPComponent; }

private:
	bool bIsAttacking = false;
	FTimerHandle AttackTimerHandle;
	
	// Level Up UI
	TSharedPtr<SLevelUpModal> ActiveLevelUpModal;

	// Network replication
	UPROPERTY(ReplicatedUsing = OnRep_Health)
	float CurrentHealth;

	UFUNCTION()
	void OnRep_Health();
};
