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

	// *** VARIÁVEIS EXPOSTAS PARA O ANIMATION BLUEPRINT ***
	
	/** Velocidade do personagem (para transições Idle/Walk/Run) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float Speed;
	
	/** Direção do movimento (-180 a 180, para Blend Space direcional) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float Direction;
	
	/** Se o personagem está no ar (para Jump/Fall) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;
	
	/** Se o personagem está acelerando (para transições suaves) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

protected:
	// Referência ao personagem
	UPROPERTY()
	AMyCharacter* MyCharacter;

	// Override dos métodos de AnimInstance
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};