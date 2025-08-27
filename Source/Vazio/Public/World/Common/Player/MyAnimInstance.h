#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MyAnimInstance.generated.h"

class AMyCharacter;

UCLASS(Blueprintable)
class VAZIO_API UMyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UMyAnimInstance();

	// *** VARI�VEIS EXPOSTAS PARA O ANIMATION BLUEPRINT ***
	
	/** Velocidade do personagem (para transi��es Idle/Walk/Run) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float Speed;
	
	/** Dire��o do movimento (-180 a 180, para Blend Space direcional) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float Direction;
	
	/** Se o personagem est� no ar (para Jump/Fall) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;
	
	/** Se o personagem est� acelerando (para transi��es suaves) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

protected:
	// Refer�ncia ao personagem
	UPROPERTY()
	AMyCharacter* MyCharacter;

	// Override dos m�todos de AnimInstance
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};