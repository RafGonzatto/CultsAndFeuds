#include "World/Common/Placement/WorldPlacer.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "World/Common/Placement/WorldPlacerConfig.h"
#include "World/Commom/Interaction/InteractableComponent.h"

AWorldPlacer::AWorldPlacer()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;
}

void AWorldPlacer::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Build(true);
}

void AWorldPlacer::BeginPlay()
{
	Super::BeginPlay();
	Build(false);
}

void AWorldPlacer::ClearBuilt()
{
	// Destroy single mesh components
	for (UStaticMeshComponent* Comp : SingleMeshComponents)
	{
		if (Comp)
		{
			Comp->DestroyComponent();
		}
	}
	SingleMeshComponents.Empty();

	// Destroy interactable components
	for (UInteractableComponent* Comp : InteractableComponents)
	{
		if (Comp)
		{
			Comp->DestroyComponent();
		}
	}
	InteractableComponents.Empty();

	// Clear HISM pools
	for (auto& Pair : HismPools)
	{
		if (Pair.Value)
		{
			Pair.Value->DestroyComponent();
		}
	}
	HismPools.Empty();
}

void AWorldPlacer::Build(bool bInEditor)
{
	ClearBuilt();
	if (!Config)
	{
		return;
	}

	for (const FWorldPlaceEntry& Entry : Config->Entries)
	{
		if (!Entry.Mesh)
		{
			continue;
		}

		// For simplicity, always create individual StaticMeshComponents here
		UStaticMeshComponent* MeshComp = CreateSingleMeshComponent(Entry);
		if (MeshComp)
		{
			AddInteractableToComponent(MeshComp, Entry);
		}
	}
}

UHierarchicalInstancedStaticMeshComponent* AWorldPlacer::GetOrCreateHISM(UStaticMesh* Mesh, bool bMovable, const FName& CollisionProfile)
{
	if (!Mesh)
	{
		return nullptr;
	}

	if (TObjectPtr<UHierarchicalInstancedStaticMeshComponent>* Found = HismPools.Find(Mesh))
	{
		return Found->Get();
	}

	UHierarchicalInstancedStaticMeshComponent* Hism = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
	// Definir mobilidade ANTES de anexar para evitar avisos de AttachTo
	Hism->SetMobility(EComponentMobility::Movable);
	Hism->SetupAttachment(RootComponent);
	Hism->SetStaticMesh(Mesh);
	Hism->SetCollisionProfileName(CollisionProfile);
	Hism->RegisterComponent();

	HismPools.Add(Mesh, Hism);
	return Hism;
}

UStaticMeshComponent* AWorldPlacer::CreateSingleMeshComponent(const FWorldPlaceEntry& Entry)
{
	UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this);
	// Sempre usar Movable para poder anexar a Root Movable e permitir ajustes
	Comp->SetMobility(EComponentMobility::Movable);
	Comp->SetupAttachment(RootComponent);
	Comp->SetStaticMesh(Entry.Mesh);
	Comp->SetCollisionProfileName(Entry.CollisionProfile);
	Comp->RegisterComponent();

	// Apply relative transform from entry
	Comp->SetRelativeTransform(Entry.Transform);

	// Propagar tag do config para o COMPONENTE do mesh (não para o ator inteiro)
	if (!Entry.Tag.IsNone())
	{
		Comp->ComponentTags.Add(Entry.Tag);
	}

	SingleMeshComponents.Add(Comp);
	return Comp;
}

void AWorldPlacer::AddInteractableToComponent(USceneComponent* AttachTo, const FWorldPlaceEntry& Entry)
{
	if (!AttachTo || !Entry.bInteractable)
	{
		return;
	}

	UInteractableComponent* Interactable = NewObject<UInteractableComponent>(this);
	Interactable->SetupAttachment(AttachTo);

	// Propagar a mesma tag para o componente de interação (compatível com GetModalClassByTag)
	if (!Entry.Tag.IsNone())
	{
		Interactable->ComponentTags.Add(Entry.Tag);
	}

	Interactable->RegisterComponent();

	InteractableComponents.Add(Interactable);
}
