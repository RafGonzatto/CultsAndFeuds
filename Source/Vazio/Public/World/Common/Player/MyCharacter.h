#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;

UCLASS()
class VAZIO_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMyCharacter();

protected:
	virtual void BeginPlay() override;

	// Câmera third-person
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	UCameraComponent* Camera;
	
	// Mesh visual como fallback (se SkeletalMesh não carregar)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh)
	UStaticMeshComponent* VisualMesh = nullptr;
};
