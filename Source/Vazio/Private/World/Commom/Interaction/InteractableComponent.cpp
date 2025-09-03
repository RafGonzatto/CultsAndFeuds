#include "World/Commom/Interaction/InteractableComponent.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "UI/Widgets/BaseModalWidget.h"
#include "UI/Widgets/InteractPromptWidget.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Character.h"

TSet<TWeakObjectPtr<UInteractableComponent>> UInteractableComponent::GRegistry;

UInteractableComponent::UInteractableComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    InitSphereRadius(200.f);
    SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SetCollisionObjectType(ECC_WorldDynamic);
    SetCollisionResponseToAllChannels(ECR_Ignore);
    SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    SetGenerateOverlapEvents(true); // garantir overlaps para prompt
    bHiddenInGame = false; // nÃ£o esconda o componente pai para nÃ£o afetar o widget filho
}

void UInteractableComponent::OnRegister()
{
	Super::OnRegister();
	GRegistry.Add(this);

	// Ajustar raio conforme EffectiveRadius
	SetSphereRadius(EffectiveRadius);

	// Bind overlaps para auto-prompt
	if (bAutoPromptOnOverlap)
	{
		OnComponentBeginOverlap.AddDynamic(this, &UInteractableComponent::OnPawnBeginOverlap);
		OnComponentEndOverlap.AddDynamic(this, &UInteractableComponent::OnPawnEndOverlap);
	}

	// Criar prompt (WidgetComponent) se classe fornecida (ou default)
    if (!PromptWidgetComponent)
    {
        TSubclassOf<UUserWidget> PromptClass = PromptWidgetClass ? PromptWidgetClass : TSubclassOf<UUserWidget>(UInteractPromptWidget::StaticClass());
        if (PromptClass)
        {
            PromptWidgetComponent = NewObject<UWidgetComponent>(GetOwner());
            PromptWidgetComponent->SetupAttachment(this);
            PromptWidgetComponent->SetWidgetClass(PromptClass);
            PromptWidgetComponent->SetDrawSize(FVector2D(64.f, 64.f));
            // Screen Space: projeÃ§Ã£o para o viewport do jogador (sempre visÃ­vel)
            PromptWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
            PromptWidgetComponent->SetDrawAtDesiredSize(true);
            if (UWorld* World = GetWorld())
            {
                if (APlayerController* PC = World->GetFirstPlayerController())
                {
                    if (ULocalPlayer* LP = PC->GetLocalPlayer())
                    {
                        PromptWidgetComponent->SetOwnerPlayer(LP);
                    }
                }
            }
            PromptWidgetComponent->SetVisibility(false);
            PromptWidgetComponent->SetHiddenInGame(true);
            PromptWidgetComponent->RegisterComponent();
            PromptWidgetComponent->SetRelativeLocation(PromptOffset);
            PromptWidgetComponent->SetPivot(FVector2D(0.5f, 1.0f));
            PromptWidgetComponent->SetDrawSize(PromptScreenSize);
            UpdatePromptTransform();
            // escala nÃ£o se aplica em Screen Space
            UE_LOG(LogTemp, Warning, TEXT("[Interactable] Prompt widget created for %s (class=%s)"), *GetOwner()->GetName(), *PromptClass->GetName());
        }
    }
}

void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	// Garanta OwnerPlayer para o WidgetComponent quando o PC existir (em BeginPlay geralmente jÃƒÂ¡ existe)
	if (PromptWidgetComponent)
	{
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				if (ULocalPlayer* LP = PC->GetLocalPlayer())
				{
					PromptWidgetComponent->SetOwnerPlayer(LP);
				}
			}
		}
	}
}

void UInteractableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GRegistry.Remove(this);
	Super::EndPlay(EndPlayReason);
}

void UInteractableComponent::ShowPrompt(bool bShow)
{
    if (PromptWidgetComponent)
    {
        const bool bCurrentlyVisible = PromptWidgetComponent->IsVisible();
        if (bCurrentlyVisible != bShow)
        {
            PromptWidgetComponent->SetVisibility(bShow);
            PromptWidgetComponent->SetHiddenInGame(!bShow);
            UE_LOG(LogTemp, Warning, TEXT("[Interactable] Prompt %s for %s"), bShow ? TEXT("VISIBLE") : TEXT("HIDDEN"), *GetOwner()->GetName());
        }
    }
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Interactable] ShowPrompt(%s) called but PromptWidgetComponent is NULL on %s"), bShow ? TEXT("true") : TEXT("false"), *GetOwner()->GetName());
	}
}

void UInteractableComponent::OnPawnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA<APawn>())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Interactable] Pawn entered radius of %s"), *GetOwner()->GetName());
		ShowPrompt(true);
	}
}

void UInteractableComponent::OnPawnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor->IsA<APawn>())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Interactable] Pawn left radius of %s"), *GetOwner()->GetName());
		ShowPrompt(false);
	}
}

void UInteractableComponent::DisablePlayerInputForModal(APlayerController* PC)
{
	if (!PC) return;
	bPrevIgnoreLook = PC->IsLookInputIgnored();
	bPrevIgnoreMove = PC->IsMoveInputIgnored();

	PC->SetIgnoreLookInput(true);
	PC->SetIgnoreMoveInput(true);

	FInputModeUIOnly Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(Mode);
	PC->bShowMouseCursor = true;
	UE_LOG(LogTemp, Warning, TEXT("[Interactable] Input disabled for modal (PC=%s)"), *PC->GetName());
}

void UInteractableComponent::RestorePlayerInput(APlayerController* PC)
{
	if (!PC) return;

	PC->SetIgnoreLookInput(bPrevIgnoreLook);
	PC->SetIgnoreMoveInput(bPrevIgnoreMove);

	FInputModeGameOnly Mode;
	PC->SetInputMode(Mode);
	PC->bShowMouseCursor = false;
	UE_LOG(LogTemp, Warning, TEXT("[Interactable] Input restored after modal (PC=%s)"), *PC->GetName());
}

void UInteractableComponent::OnModalClosed()
{
	ActiveModal = nullptr;
	if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Interactable] Modal closed for %s"), *GetOwner()->GetName());
		RestorePlayerInput(PC);
		ShowPrompt(true);
	}
}

void UInteractableComponent::Interact_Implementation(APlayerController* InteractingPC)
{
	if (!InteractingPC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Interactable] Interact called but PC is NULL for %s"), *GetOwner()->GetName());
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Interactable] Interact pressed on %s by %s"), *GetOwner()->GetName(), *InteractingPC->GetName());
	TSubclassOf<UUserWidget> ClassToUse = ModalWidgetClass ? TSubclassOf<UUserWidget>(*ModalWidgetClass) : TSubclassOf<UUserWidget>(UBaseModalWidget::StaticClass());
    if (UBaseModalWidget* Modal = CreateWidget<UBaseModalWidget>(InteractingPC, ClassToUse))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Interactable] Modal created (%s) for %s"), *ClassToUse->GetName(), *GetOwner()->GetName());
        DisablePlayerInputForModal(InteractingPC);
        Modal->OnModalClosed.AddDynamic(this, &UInteractableComponent::OnModalClosed);
        Modal->AddToViewport(1000);
        // Garantir foco do input de UI no modal aberto
        {
            FInputModeUIOnly FocusMode;
            FocusMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            FocusMode.SetWidgetToFocus(Modal->TakeWidget());
            InteractingPC->SetInputMode(FocusMode);
            InteractingPC->bShowMouseCursor = true;
        }
        ShowPrompt(false);
        Modal->FocusFirstWidget();
        UE_LOG(LogTemp, Warning, TEXT("[Interactable] Modal added to viewport and focused for %s"), *GetOwner()->GetName());
    }
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Interactable] CreateWidget failed for %s (class=%s)"), *GetOwner()->GetName(), *ClassToUse->GetName());
	}
}

void UInteractableComponent::UpdatePromptTransform()
{
	if (!PromptWidgetComponent) return;

	// Garantir tamanho/pivô desejados
	PromptWidgetComponent->SetDrawSize(PromptScreenSize);
	PromptWidgetComponent->SetPivot(FVector2D(0.5f, 1.0f)); // centro em X, base em Y

	// Calcular posição desejada: topo do mesh pai + margem vertical
	UPrimitiveComponent* ParentPrim = Cast<UPrimitiveComponent>(GetAttachParent());
	if (!ParentPrim)
	{
		// fallback para o offset simples relativo ao próprio componente
		PromptWidgetComponent->SetRelativeLocation(PromptOffset);
		return;
	}

	const FBoxSphereBounds ParentBounds = ParentPrim->Bounds; // world space
	const FVector WorldTop = ParentBounds.Origin + FVector(0, 0, ParentBounds.BoxExtent.Z + PromptWorldMargin);

	// Converter para o espaço relativo deste componente (pois o widget é filho deste componente)
	const FTransform ThisXform = GetComponentTransform();
	const FVector RelToThis = ThisXform.InverseTransformPosition(WorldTop);
	PromptWidgetComponent->SetRelativeLocation(RelToThis);
}