#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "World/Common/Interaction/Interactable.h"
#include "InteractableComponent.generated.h"

class UWidgetComponent;
class UUserWidget;
class UBaseModalWidget;
class APlayerController;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VAZIO_API UInteractableComponent : public USphereComponent, public IInteractable
{
	GENERATED_BODY()

public:
	UInteractableComponent();

	// Registro global de interactables para descoberta simples pelo PlayerController
	static TSet<TWeakObjectPtr<UInteractableComponent>> GRegistry;

	// Raio efetivo usado para detectar proximidade e ajustar o prompt
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction", meta=(ClampMin="0.0"))
	float EffectiveRadius = 200.f;

	// Mostrar/ocultar prompt automaticamente em overlaps
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction|Prompt")
	bool bAutoPromptOnOverlap = true;

	// Classe do widget para o prompt de intera��o
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction|Prompt")
	TSubclassOf<UUserWidget> PromptWidgetClass;

	// Offset relativo do prompt
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction|Prompt")
	FVector PromptOffset = FVector(0.f, 0.f, 80.f);

	// Tamanho do prompt (px) quando usado em Screen Space
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction|Prompt")
	FVector2D PromptScreenSize = FVector2D(50.f, 50.f);

	// Margem vertical acima do topo do mesh (cm no mundo)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction|Prompt")
	float PromptWorldMargin = 20.f;

	// Classe do modal a ser aberto quando interagir
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction|Modal")
	TSubclassOf<UBaseModalWidget> ModalWidgetClass;

	// Inst�ncia ativa do modal, se existir
	UPROPERTY(Transient)
	TObjectPtr<UBaseModalWidget> ActiveModal = nullptr;

	// Componente do widget do prompt
	UPROPERTY(Transient)
	TObjectPtr<UWidgetComponent> PromptWidgetComponent = nullptr;

	// Exibir/ocultar prompt
	UFUNCTION(BlueprintCallable, Category="Interaction|Prompt")
	void ShowPrompt(bool bShow);

	// IInteractable
	virtual void Interact_Implementation(APlayerController* InteractingPC) override;

protected:
	// UActorComponent / USceneComponent overrides
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Atualiza a posição/escala/pivô do widget conforme o mesh pai
	void UpdatePromptTransform();

	// Overlaps para auto-prompt
	UFUNCTION()
	void OnPawnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnPawnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Modal helpers
	void DisablePlayerInputForModal(APlayerController* PC);
	void RestorePlayerInput(APlayerController* PC);

	UFUNCTION()
	void OnModalClosed();

	// Cache do estado de input anterior
	bool bPrevIgnoreLook = false;
	bool bPrevIgnoreMove = false;
};
