#include "Swarm/SwarmSubsystem.h"
#include "Swarm/SwarmManager.h"
#include "Swarm/SwarmVisualComponent.h"
#include "SwarmProjectilePool.h"
#include "Swarm/SwarmVisibilityComponent.h"
#include "Swarm/SwarmSpawnRules.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include <random>
#include "Swarm/Core/World.h"
#include "Algo/Accumulate.h"
using namespace SwarmCore;


void USwarmSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogSwarm, Display, TEXT("SwarmSubsystem Initialize (World=%s)"), *GetWorld()->GetName());
}

void USwarmSubsystem::Deinitialize()
{
    if (Core) { delete Core; Core = nullptr; }
    UE_LOG(LogSwarm, Display, TEXT("SwarmSubsystem Deinitialize (World=%s)"), GetWorld() ? *GetWorld()->GetName() : TEXT("<null>"));
    Super::Deinitialize();
}

bool USwarmSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void USwarmSubsystem::InitWithConfig(USwarmConfig* InCfg, ASwarmManager* InMgr) {
	Cfg = InCfg; Manager = InMgr; if (!Cfg || !Manager.IsValid()) { UE_LOG(LogSwarm, Warning, TEXT("Missing Config/Manager. Place a SwarmManager in the level and set its 'Config' to your USwarmConfig DataAsset (e.g. 'DA_SwarmConfig').")); return; }
	if (Cfg->EnemyTypes.Num() == 0) { UE_LOG(LogSwarm, Warning, TEXT("SwarmSubsystem: Config '%s' has 0 EnemyTypes; no enemies will be spawned."), *Cfg->GetName()); }
	Params P{ Cfg->CellSize,Cfg->Separation,Cfg->MaxSpeed,Cfg->WorldExtent };
	P.LockZ = Cfg->bLockAgentsToGround; P.GroundZ = Cfg->GroundZHeight; P.ChaseTarget = true; P.ChaseAccel = 600.f;
	std::vector<EnemyType> T; T.reserve(Cfg->EnemyTypes.Num()); for (auto& e : Cfg->EnemyTypes) { T.push_back({ e.Radius,e.Speed,e.HP,e.DPS }); }
	Core = new World(); Core->Configure(P, T, Cfg->ProjectilePool, Cfg->ProjectileSpeed, Cfg->ProjectileRadius); Manager->BindCore(Core); BuildVisuals();

	// Hook visibility component if present
	if (ASwarmManager* M = Manager.Get()) { Visibility = M->GetVisibility(); }

	// Progressive spawn: set pending per type, spawn per frame respeitando orçamento e visibilidade
	PendingSpawnPerType.SetNum(Cfg->EnemyTypes.Num());
	for (int32 t = 0; t < Cfg->EnemyTypes.Num(); ++t) { PendingSpawnPerType[t] = Manager->AutoSpawnCount; }
	UE_LOG(LogSwarm, Display, TEXT("SwarmSubsystem: Queued %d per type across %d types for progressive in-view spawn."), Manager->AutoSpawnCount, Cfg->EnemyTypes.Num());
}


void USwarmSubsystem::BuildVisuals() { if (!Manager.IsValid() || !Cfg) return; Manager->GetVisual()->BuildFromConfig(*Cfg); }


void USwarmSubsystem::Tick(float Dt) {
    UE_LOG(LogSwarm, VeryVerbose, TEXT("[SwarmTick] Tick dt=%.3f"), Dt);
    if (!Manager.IsValid())
    {
        for (TActorIterator<ASwarmManager> It(GetWorld()); It; ++It) { Manager = *It; break; }
        if (!Manager.IsValid())
        {
            UE_LOG(LogSwarm, VeryVerbose, TEXT("[SwarmTick] Waiting for SwarmManager"));
            return;
        }
        if (!Cfg)
        {
            Cfg = Manager->Config;
            if (!Cfg)
            {
                UE_LOG(LogSwarm, Warning, TEXT("[SwarmTick] Manager has no Config yet"));
                return;
            }
        }
        UE_LOG(LogSwarm, Display, TEXT("[SwarmTick] Manager ready: %s"), *Manager->GetName());
    }
    if (!Core)
    {
        InitWithConfig(Cfg, Manager.Get());
        UE_LOG(LogSwarm, Display, TEXT("[SwarmTick] Core initialized"));
        return; // proceed on next tick
    }

    // Feed chase target as player/camera location
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APlayerCameraManager* Cam = PC->PlayerCameraManager)
        {
            Core->TargetX = Cam->GetCameraLocation().X;
            Core->TargetY = Cam->GetCameraLocation().Y;
            Core->TargetZ = Cfg->GroundZHeight;
        }
    }

	// Progressive in-view spawn
    int32 SpawnedThisFrame = 0;
    if (Cfg && Cfg->bSpawnInViewOnly && PendingSpawnPerType.Num() == Cfg->EnemyTypes.Num())
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        APlayerCameraManager* Cam = PC ? PC->PlayerCameraManager : nullptr;
        if (Cam)
        {
            int32 Budget = Cfg->MaxSpawnPerFrame;
            int32 Rays = Cfg->MaxRaycastPerFrame;
            static int32 DebugFrames = 240; // 4s de logs de alto nível
            if (DebugFrames > 0)
            {
                UE_LOG(LogSwarm, Warning, TEXT("[SpawnDbg] Budget=%d Rays=%d PendingTotal=%d"), Budget, Rays, PendingSpawnPerType.Num()>0 ? Algo::Accumulate(PendingSpawnPerType, 0) : 0);
            }
            for (int32 t = 0; t < PendingSpawnPerType.Num() && Budget > 0; ++t)
            {
                if (PendingSpawnPerType[t] <= 0) continue;
                int32 ToSpawn = FMath::Min(PendingSpawnPerType[t], Budget);
                for (int32 i = 0; i < ToSpawn; ++i)
                {
                    FVector Candidate;
                    const bool bForceOut = Cfg->EnemyTypes.IsValidIndex(t) ? Cfg->EnemyTypes[t].bForceSpawnOutOfView : false;
                    bool bSpawned = false;
                    const int32 MaxAttempts = 8;
                    for (int32 attempt = 0; attempt < MaxAttempts && !bSpawned; ++attempt)
                    {
                        if (bForceOut)
                        {
                            const FVector Fwd = Cam->GetCameraRotation().Vector();
                            const FVector Right = FRotationMatrix(Cam->GetCameraRotation()).GetUnitAxis(EAxis::Y);
                            FRandomStream Rng(FPlatformTime::Cycles() + attempt);
                            const float Dist = Rng.FRandRange(Cfg->NearSpawn, Cfg->FarSpawn);
                            const FVector Disk = FVector(Rng.FRandRange(-1.f,1.f), Rng.FRandRange(-1.f,1.f), 0.f).GetClampedToMaxSize(1.f) * Cfg->FOVSpawnRadius;
                            Candidate = Cam->GetCameraLocation() - Fwd * Dist + Right * Disk.X + FVector::UpVector * Disk.Y;
                        }
                        else
                        {
                            SwarmSpawn::FindSpawnPointInView(*GetWorld(), Cam->GetCameraLocation(), Cam->GetCameraRotation(), Cfg->NearSpawn, Cfg->FarSpawn, Cfg->FOVSpawnRadius, Candidate);
                        }

                        const FVector PlayerPos = Cam->GetCameraLocation();
                        if (FVector::DistSquared(PlayerPos, Candidate) < FMath::Square(Cfg->MinSpawnDistanceFromPlayer))
                        {
                            continue;
                        }

                        FVector FinalP = Candidate;
                        bool bOk = true;
                        if (Rays > 0)
                        {
                            bOk = SwarmSpawn::ProjectToNavmeshOrGround(*GetWorld(), Candidate, Cfg->UseNavmeshProjection, FinalP);
                            Rays--;
                        }
                        if (bOk)
                        {
                            if (Cfg->bLockAgentsToGround)
                            {
                                FinalP.Z = Cfg->GroundZHeight;
                            }
                            Core->Spawn((type_t)t, FinalP.X, FinalP.Y, FinalP.Z);
                            PendingSpawnPerType[t]--;
                            SpawnedThisFrame++;
                            bSpawned = true;
                            if (DebugFrames > 0)
                            {
                                UE_LOG(LogSwarm, Warning, TEXT("[SpawnDbg] OK t=%d p=(%.0f,%.0f,%.0f)"), t, FinalP.X, FinalP.Y, FinalP.Z);
                            }
                        }
                        else if (DebugFrames > 0)
                        {
                            UE_LOG(LogSwarm, Warning, TEXT("[SpawnDbg] FAIL t=%d attempt=%d cand=(%.0f,%.0f,%.0f)"), t, attempt+1, Candidate.X, Candidate.Y, Candidate.Z);
                        }
                    }
                }
                Budget -= ToSpawn;
            }
            if (DebugFrames > 0) { DebugFrames--; }
        }
    }

	TRACE_CPUPROFILER_EVENT_SCOPE(SwarmStep);
	Core->Step(Dt);

	// Visibility/culling
    if (Cfg && (Cfg->FrustumCull || Cfg->DistanceCull) && Visibility)
    {
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            if (APlayerCameraManager* Cam = PC->PlayerCameraManager)
            {
                const FVector CamPos = Cam->GetCameraLocation();
                const FVector CamFwd = Cam->GetActorForwardVector();
                const float FOV = Cam->GetFOVAngle();
                Visibility->Compute(*Core, CamPos, CamFwd, FOV, *Cfg);
            }
        }
    }

	StepVisuals();
	Manager->GetProjPool()->SyncFromCore(Core->Proj);

    // Periodic count log every 5 seconds
    if (UWorld* World = GetWorld())
    {
        const double Now = World->GetTimeSeconds();
        if (Now - LastCountLogAt >= 5.0)
        {
            const int Types = Cfg ? Cfg->EnemyTypes.Num() : 0;
            TArray<int32> PerType; PerType.Init(0, Types);
            int32 AliveCount = 0;
            const int N = (int)Core->Alive.size();
            for (int i = 0; i < N; ++i)
            {
                if (!Core->Alive[i]) continue;
                AliveCount++;
                const int t = (int)Core->Type[i];
                if (PerType.IsValidIndex(t)) PerType[t]++;
            }
            FString Detail;
            for (int t = 0; t < Types; ++t)
            {
                Detail += FString::Printf(TEXT(" t%d=%d"), t, PerType[t]);
            }
            UE_LOG(LogSwarm, Display, TEXT("Swarm Count: Alive=%d Types=%d%s | SpawnedThisFrame=%d | Pending=%d"), AliveCount, Types, *Detail, SpawnedThisFrame, PendingSpawnPerType.Num()>0 ? Algo::Accumulate(PendingSpawnPerType, 0) : 0);
            LastCountLogAt = Now;
        }
    }
}


void USwarmSubsystem::StepVisuals() { if (!Cfg || !Manager.IsValid()) return; auto* Vis = Manager->GetVisual(); const int types = Cfg->EnemyTypes.Num(); for (int t = 0; t < types; ++t) { TArray<FTransform> X; X.Reserve(1024); if (Cfg->FrustumCull || Cfg->DistanceCull) { if (Visibility) { const TArray<int32>& Indices = Visibility->GetVisibleOfType(t); X.Reserve(Indices.Num()); for (int32 i : Indices) { const FVector p(Core->Px[i], Core->Py[i], Core->Pz[i]); const FVector v(Core->Vx[i], Core->Vy[i], Core->Vz[i]); const FRotator r = v.IsNearlyZero() ? FRotator::ZeroRotator : v.Rotation(); X.Add(FTransform(r, p)); } } } else { const int N = (int)Core->Px.size(); for (int i = 0; i < N; ++i) { if (!Core->Alive[i] || Core->Type[i] != t) continue; const FVector p(Core->Px[i], Core->Py[i], Core->Pz[i]); const FVector v(Core->Vx[i], Core->Vy[i], Core->Vz[i]); const FRotator r = v.IsNearlyZero() ? FRotator::ZeroRotator : v.Rotation(); X.Add(FTransform(r, p)); } } Vis->UpdateTypeTransforms(t, X); } }


