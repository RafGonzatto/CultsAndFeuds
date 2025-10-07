// AutoRespawnComponent.cpp
#include "Core/Flow/AutoRespawnComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/TriggerBox.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"

UAutoRespawnComponent::UAutoRespawnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAutoRespawnComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!IsValid(GetOwner())) return;

	// Executa apenas no servidor (SP/Listen/Dedicated)
	if (GetOwner()->HasAuthority())
	{
		CachedSpawnPoint = ResolveSpawnPoint();

		if (bTryFindBoundsVolume)
		{
			for (TActorIterator<ATriggerBox> It(GetWorld()); It; ++It)
			{
				if (It->ActorHasTag(TEXT("RespawnBounds")))
				{
					CachedBounds = *It;
					break;
				}
			}
		}
		StartChecks();
	}
}

void UAutoRespawnComponent::StartChecks()
{
	if (CheckIntervalSeconds <= 0.f) CheckIntervalSeconds = 0.2f;
	GetWorld()->GetTimerManager().SetTimer(
		CheckHandle, this, &UAutoRespawnComponent::CheckAndRecover,
		CheckIntervalSeconds, true
	);
}

void UAutoRespawnComponent::CheckAndRecover()
{
	AActor* Owner = GetOwner(); if (!Owner) return;

	const FVector Loc = Owner->GetActorLocation();
	if (!IsOutOfBounds(Loc)) return;

	AActor* Spawn = OverrideSpawnPoint.IsValid() ? OverrideSpawnPoint.Get() : CachedSpawnPoint.Get();
	if (!Spawn)
	{
		CachedSpawnPoint = ResolveSpawnPoint();
		Spawn = CachedSpawnPoint.Get();
	}
	TeleportOwnerToSpawn(Spawn);
}

bool UAutoRespawnComponent::IsOutOfBounds(const FVector& Loc) const
{
	// 1) Volume com tag "RespawnBounds" (preferido se existir)
	if (CachedBounds.IsValid())
	{
		if (const UBoxComponent* Box = Cast<UBoxComponent>(CachedBounds->GetCollisionComponent()))
		{
			const FTransform T = CachedBounds->GetActorTransform();
			const FVector Local = T.InverseTransformPosition(Loc);
			const FVector Ext = Box->GetUnscaledBoxExtent();

			// Considera um "colar" extra em X/Y para permitir sair lateralmente sem punir; foco � Z
			const bool bOutZ = (Local.Z < -Ext.Z) || (Local.Z > Ext.Z);
			return bOutZ;
		}
	}

	// 2) Limites num�ricos de Z
	if (bUseNumericZLimits)
	{
		return (Loc.Z < MinZ) || (Loc.Z > MaxZ);
	}

	// 3) Fallback: usa WorldSettings->KillZ s� para abaixo
	if (const AWorldSettings* WS = GetWorld()->GetWorldSettings())
	{
		if (Loc.Z < WS->KillZ) return true;
	}

	return false;
}

AActor* UAutoRespawnComponent::ResolveSpawnPoint() const
{
	// 1) PlayerStart com tag preferida
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag(PreferredSpawnTag)) return *It;
	}
	// 2) Qualquer PlayerStart
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		return *It;
	}
	// 3) Falhou
	return nullptr;
}
bool UAutoRespawnComponent::TeleportOwnerToSpawn(AActor* Spawn) const
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner)) return false;

	// Resolve destino
	FVector TargetLoc = FVector(0, 0, 150);
	FRotator TargetRot = FRotator::ZeroRotator;
	if (Spawn)
	{
		TargetLoc = Spawn->GetActorLocation();
		TargetRot = Spawn->GetActorRotation();
	}

	// (1) tenta achar spot v�lido para Pawn
	if (APawn* Pawn = Cast<APawn>(Owner))
	{
		// opcional: tenta �snap to ground� primeiro
		UWorld* W = GetWorld();
		FHitResult Hit;
		const FVector Start = TargetLoc + FVector(0, 0, 2000);
		const FVector End = TargetLoc - FVector(0, 0, 5000);
		FCollisionQueryParams Q(SCENE_QUERY_STAT(AutoRespawn_Ground), false, nullptr);

		if (W && W->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Q))
		{
			TargetLoc = Hit.ImpactPoint + FVector(0, 0, 50); // 50cm acima do ch�o
		}

		// garante parada
		if (ACharacter* C = Cast<ACharacter>(Pawn))
		{
			if (UCharacterMovementComponent* Move = C->GetCharacterMovement())
			{
				Move->StopMovementImmediately();
				Move->Velocity = FVector::ZeroVector;
			}
		}

		// tenta spot sem overlap
		FVector TryLoc = TargetLoc;
		FRotator TryRot = TargetRot;

		if (W && W->FindTeleportSpot(Pawn, TryLoc, TryRot))
		{
			return Pawn->TeleportTo(TryLoc, TryRot, /*bIsATest*/false, /*bNoCheck*/false);
		}

		// pequenos �lifts� em Z antes do no-check
		for (int i = 0; i < 5; ++i)
		{
			FVector LiftLoc = TargetLoc + FVector(0, 0, 50.f * (i + 1));
			if (W && W->FindTeleportSpot(Pawn, LiftLoc, TryRot))
			{
				return Pawn->TeleportTo(LiftLoc, TryRot, false, false);
			}
		}

		// �ltimo recurso: no-check (evita stucks raros em mapas vazios)
		return Pawn->TeleportTo(TargetLoc, TargetRot, /*bIsATest*/false, /*bNoCheck*/true);
	}

	// (2) n�o-Pawn (raro): teleporta direto
	return Owner->TeleportTo(TargetLoc, TargetRot, false, true);
}