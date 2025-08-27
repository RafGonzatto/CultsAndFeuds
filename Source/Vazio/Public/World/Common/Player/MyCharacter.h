#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;
class USkeletalMesh;
class UAnimInstance;

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
