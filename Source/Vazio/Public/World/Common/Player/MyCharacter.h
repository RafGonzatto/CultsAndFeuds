#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "World/Common/Player/PlayerHealthComponent.h"
#include "World/Common/Player/XPComponent.h"
#include "MyCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;
class USkeletalMesh;
class UAnimInstance;
class USphereComponent;

UCLASS()
class VAZIO_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMyCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaTime) override;

	// Câmera top-down
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	USpringArmComponent* SpringArm = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	UCameraComponent* Camera = nullptr;

	// Classe do Animation Blueprint a ser usada em runtime (defina no BP)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom Character|Animation")
	TSubclassOf<UAnimInstance> AnimBPClass;

	// SISTEMA DE TAUNTS ÚNICO - SEM FALLBACKS
	UPROPERTY(Transient)
	UAnimMontage* CurrentTauntMontage = nullptr;
	float TauntInterruptGraceEndTime = 0.f;
	bool bTauntWasSuccessfullyStarted = false;
	
	UFUNCTION()
	void OnTauntMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// DEBUG AVANÇADO
	FVector LastTickPosition;
	float DebugUpdateTimer = 0.f;
	int32 PositionJumpCount = 0;
	
	void DebugPositionTracking();
	void ValidateComponentAlignment();

	// Handler de morte do jogador
	UFUNCTION()
	void OnPlayerDeath();

public:
	/** --------------------  NEW: Health & XP  -------------------- */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG")
	UPlayerHealthComponent* Health = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG")
	UXPComponent* XP = nullptr;

	/** Ataque em área (chamado pelo PlayerController) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PerformAreaAttack(float Radius, float Damage);

private:
	/** Dano por segundo causado por inimigos em contato */
	UPROPERTY(EditAnywhere, Category = "Combat") float ContactDPS = 5.f;

	/** Sensor simples para detectar inimigos próximos (apenas overlap) */
	UPROPERTY(VisibleAnywhere, Category = "Combat") USphereComponent* DamageSense = nullptr;

	/** Conjunto de inimigos atualmente em contato */
	TSet<TWeakObjectPtr<AActor>> ContactingEnemies;

	UFUNCTION()
	void OnSenseBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnSenseEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	// Assets de configuração
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom Character")
	USkeletalMesh* CustomSkeletalMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom Character")
	UAnimMontage* AnimMontage1 = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom Character")
	UAnimMontage* AnimMontage2 = nullptr;

	// SISTEMA ÚNICO E DEFINITIVO - SEM FALLBACKS
	UFUNCTION(BlueprintCallable, Category = "Custom Character")
	void PlayAnim1();

	UFUNCTION(BlueprintCallable, Category = "Custom Character")
	void PlayAnim2();

	UFUNCTION(BlueprintCallable, Category = "Custom Character")
	void InterruptTaunt(float BlendOutTime = 0.2f);
	
	UFUNCTION(BlueprintCallable, Category = "Custom Character")
	bool HasActiveTaunt() const;

	// COMANDOS DE DEBUG
	UFUNCTION(Exec)
	void CharDump();
	
	UFUNCTION(Exec)
	void DebugRootMotion();
	
	UFUNCTION(Exec)
	void ForceIgnoreRootMotion();
	
	UFUNCTION(Exec)
	void ValidateSetup();
};
