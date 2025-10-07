#include "Enemy/EnemyBase.h"
#include "Enemy/Components/EnemyDropComponent.h"
#include "Enemy/Components/EnemyAuraComponent.h"
#include "World/Common/Player/MyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "TimerManager.h"
#include "Enemy/EnemyTypes.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h" // For TActorIterator
#include "Components/PointLightComponent.h"

AEnemyBase::AEnemyBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create components
    DropComponent = CreateDefaultSubobject<UEnemyDropComponent>(TEXT("DropComponent"));
    AuraComponent = CreateDefaultSubobject<UEnemyAuraComponent>(TEXT("AuraComponent"));

    // Create visual mesh component
    VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
    VisualMesh->SetupAttachment(RootComponent);
    
    // Add a small point light to help see enemies easily
    DebugLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("DebugLight"));
    DebugLight->SetupAttachment(VisualMesh);
    DebugLight->Intensity = 4000.f;
    DebugLight->AttenuationRadius = 200.f;
    DebugLight->bUseInverseSquaredFalloff = false;
    DebugLight->LightColor = FColor::Red;
    
    // Set default cube mesh for visibility
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        VisualMesh->SetStaticMesh(CubeMeshAsset.Object);
        VisualMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 2.0f)); // Make it taller like a character
        VisualMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f)); // Center it
    }

    // Configure collision - ENEMIES PASS THROUGH PLAYER, DEAL DAMAGE ON OVERLAP
    // Use OverlapAllDynamic profile to match player's DamageSphere
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    GetCapsuleComponent()->SetGenerateOverlapEvents(true); // CRITICAL: Enable overlap events
    
    // PRECISE COLLISION - Much smaller for accurate damage area
    GetCapsuleComponent()->SetCapsuleRadius(25.0f); // Very small radius for precise collision
    GetCapsuleComponent()->SetCapsuleHalfHeight(45.0f); // Smaller height too
    
    // Set collision for VisualMesh to NOT generate overlaps - only capsule should
    if (VisualMesh)
    {
        VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // VisualMesh doesn't need collision
        VisualMesh->SetGenerateOverlapEvents(false); // Only capsule generates overlaps
    }

    // Configure movement
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->MaxWalkSpeed = 300.f;
        MovementComp->bOrientRotationToMovement = true;
        MovementComp->RotationRate = FRotator(0.f, 540.f, 0.f);
    }

    // Initialize default values
    CurrentHP = 100.f;
    MaxHP = 100.f;
    bIsParent = false;
}

void AEnemyBase::BeginPlay()
{
    Super::BeginPlay();
    
    // GARANTIR QUE TICK ESTÁ HABILITADO
    PrimaryActorTick.bCanEverTick = true;
    SetActorTickEnabled(true);
    
    UE_LOG(LogTemp, Warning, TEXT("[ENEMY] %s BeginPlay - Tick Enabled: %s"), 
        *GetName(), IsActorTickEnabled() ? TEXT("YES") : TEXT("NO"));
    
    SetupMovement();
    // Garantir que o componente de movimento está ativo e em modo Walking
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->Activate(true);
        MovementComp->SetComponentTickEnabled(true);
        if (MovementComp->MovementMode == MOVE_None)
        {
            MovementComp->SetMovementMode(MOVE_Walking);
        }
    }
    
    // Setup damage on overlap - ONLY CapsuleComponent for precise collision
    GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AEnemyBase::OnOverlapBegin);
    
    // Force initial color application if not already done
    if (VisualMesh)
    {
        // Apply default archetype values if not set
        if (CurrentArchetype.BaseDMG <= 0.0f)
        {
            CurrentArchetype.BaseDMG = 25.0f; // Default damage
            CurrentArchetype.BaseSpeed = 300.0f; // Default speed
        }
        
        // Force color application
        UMaterialInstanceDynamic* DynamicMaterial = VisualMesh->CreateAndSetMaterialInstanceDynamic(0);
        if (DynamicMaterial)
        {
            FLinearColor EnemyColor = FLinearColor::Red;
            FString ClassName = GetClass()->GetName();
            
            if (ClassName.Contains(TEXT("Heavy"))) EnemyColor = FLinearColor::Blue;
            else if (ClassName.Contains(TEXT("Ranged"))) EnemyColor = FLinearColor::Yellow;
            else if (ClassName.Contains(TEXT("Dash"))) EnemyColor = FLinearColor::Green;
            
            DynamicMaterial->SetVectorParameterValue(FName("BaseColor"), EnemyColor);
            DynamicMaterial->SetScalarParameterValue(FName("EmissiveStrength"), 5.0f);
            // Update light color to match
            if (DebugLight)
            {
                DebugLight->SetLightColor(EnemyColor.ToFColor(true));
            }
            UE_LOG(LogTemp, Warning, TEXT("[BEGINPLAY] %s: Applied initial color %s"), *GetName(), *EnemyColor.ToString());
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[BEGINPLAY] %s: HP=%.1f, Speed=%.1f, Damage=%.1f"), 
        *GetName(), CurrentHP, GetCharacterMovement()->MaxWalkSpeed, CurrentArchetype.BaseDMG);

    // Initialize movement logging baseline
    PreviousLocation = GetActorLocation();
    bHasPreviousLocation = true;
    LastTargetLocation = PreviousLocation; // will be updated next ChasePlayer
}

void AEnemyBase::Tick(float DeltaTime)
{
    // SAFETY: Always check validity first
    if (!IsValid(this))
    {
        return;
    }
    
    Super::Tick(DeltaTime);

    if (!bFirstTickLogged)
    {
        UE_LOG(LogTemp, Warning, TEXT("[TICK-ONCE] %s primeiro Tick executado"), *GetName());
        bFirstTickLogged = true;
    }
    
    // SAFE movement execution with try-catch equivalent (validation checks)
    bool bCanMove = true;
    
    // Check if we're in a valid state for movement
    if (CurrentModifiers.bImmovable)
    {
        bCanMove = false;
    }
    
    // Check world validity
    UWorld* World = GetWorld();
    if (!World || !IsValid(World))
    {
        bCanMove = false;
    }
    
    // Check movement component validity
    UCharacterMovementComponent* MovementComp = GetCharacterMovement();
    if (!MovementComp || !IsValid(MovementComp))
    {
        bCanMove = false;
    }
    
    // Only attempt base chase if enabled and all checks pass
    if (bCanMove && bUseBaseChase)
    {
        ChasePlayer();
    }
    
    // SAFE performance optimization call
    if (World && IsValid(World))
    {
        PerformanceOptimization();
    }
    
    // SAFE dash logic call
    HandleDashLogic(DeltaTime);
    
    // DEBUG: Very occasional logging to confirm tick is working
    static TMap<AEnemyBase*, float> TickLogTimes;
    if (World)
    {
        float CurrentTime = World->GetTimeSeconds();
        float* LastTickLogPtr = TickLogTimes.Find(this);
        
        if (!LastTickLogPtr || (CurrentTime - *LastTickLogPtr > 5.0f))
        {
            UE_LOG(LogTemp, VeryVerbose, TEXT("[TICK] %s ticking safely - HP=%.1f, CanMove=%s"), 
                *GetName(), CurrentHP, bCanMove ? TEXT("YES") : TEXT("NO"));
            TickLogTimes.Add(this, CurrentTime);
        }
    }
}

// Console helper para reativar ticks em todos os inimigos (caso algum blueprint tenha desabilitado)
static FAutoConsoleCommand CmdEnemyForceTick(
    TEXT("Enemy.ForceTick"),
    TEXT("Reativa SetActorTickEnabled(true) e TickInterval=0 para todos AEnemyBase vivos"),
    FConsoleCommandDelegate::CreateStatic([]()
    {
        if (GWorld)
        {
            int32 Count = 0;
            for (TActorIterator<AEnemyBase> It(GWorld); It; ++It)
            {
                AEnemyBase* E = *It;
                if (E && IsValid(E))
                {
                    E->SetActorTickEnabled(true);
                    E->PrimaryActorTick.bCanEverTick = true;
                    E->PrimaryActorTick.TickInterval = 0.f;
                    ++Count;
                }
            }
            UE_LOG(LogTemp, Warning, TEXT("[Enemy.ForceTick] Reativados %d inimigos"), Count);
        }
    }),
    ECVF_Default
);

void AEnemyBase::HandleDeath(bool bIsParentParam)
{
    UE_LOG(LogEnemy, Log, TEXT("%s died (parent=%d)"), *GetName(), bIsParentParam ? 1 : 0);

    // Handle drops through DropComponent
    if (DropComponent)
    {
        DropComponent->DropOnDeath(this, CurrentArchetype, CurrentModifiers, bIsParentParam);
    }

    // Clear any active timers
    if (DissolveTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(DissolveTimerHandle);
    }
    if (DashCooldownHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(DashCooldownHandle);
    }

    // Trigger death effects
    OnStartDissolve();

    // Return to pool or destroy
    // TODO: Integrate with pool system
    Destroy();
}

void AEnemyBase::ApplyArchetypeAndModifiers(const FEnemyArchetype& Arch, const FEnemyInstanceModifiers& Mods)
{
    CurrentArchetype = Arch;
    CurrentModifiers = Mods;

    // 1. Apply base archetype stats
    MaxHP = Arch.BaseHP;
    CurrentHP = MaxHP;
    
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->MaxWalkSpeed = Arch.BaseSpeed;
    }

    // Apply mesh scale based on BaseSize - NORMAL PLAYER-SIZE SCALE
    if (VisualMesh)
    {
        // NORMAL SCALE - Same size as player character (about 180cm tall)
        float NormalScale = 0.9f; // Slightly smaller than player for visibility
        VisualMesh->SetWorldScale3D(FVector(NormalScale, NormalScale, NormalScale * 1.8f)); 
        
        // KEEP ON GROUND LEVEL - No elevation, spawn at proper ground level
        FVector CurrentLocation = GetActorLocation();
        // Only adjust Z if we're way off the ground
        if (CurrentLocation.Z > 400.0f || CurrentLocation.Z < -100.0f)
        {
            SetActorLocation(FVector(CurrentLocation.X, CurrentLocation.Y, 90.0f)); // Standard ground level
        }
        
        UE_LOG(LogTemp, Warning, TEXT("[DEBUG] Enemy %s: Location=%s, Scale=%s"), 
            *GetName(), 
            *GetActorLocation().ToString(),
            *VisualMesh->GetComponentScale().ToString());
        
        // CRASH-SAFE MATERIAL SYSTEM
        if (VisualMesh && IsValid(VisualMesh))
        {
            UMaterialInterface* BaseMaterial = nullptr;
            
            // Try multiple material sources with safe loading
            TArray<FString> MaterialPaths = {
                TEXT("/Engine/EngineMaterials/WorldGridMaterial"),
                TEXT("/Engine/EngineMaterials/DefaultMaterial"),
                TEXT("/Engine/BasicShapes/BasicShapeMaterial")
            };
            
            for (const FString& Path : MaterialPaths)
            {
                BaseMaterial = LoadObject<UMaterialInterface>(nullptr, *Path);
                if (BaseMaterial && IsValid(BaseMaterial))
                {
                    UE_LOG(LogTemp, Warning, TEXT("[MATERIAL] %s: Successfully loaded %s"), *GetName(), *Path);
                    break;
                }
            }
            
            // Apply material if found
            if (BaseMaterial && IsValid(BaseMaterial))
            {
                VisualMesh->SetMaterial(0, BaseMaterial);
                UE_LOG(LogTemp, Warning, TEXT("[MATERIAL] %s: Applied base material"), *GetName());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("[MATERIAL] %s: All material loading failed - using default"), *GetName());
            }
        }
        
        // CRASH-SAFE DYNAMIC MATERIAL CREATION
        if (VisualMesh && IsValid(VisualMesh))
        {
            // Safe dynamic material creation with extensive validation
            UMaterialInstanceDynamic* DynamicMaterial = VisualMesh->CreateAndSetMaterialInstanceDynamic(0);
            
            if (DynamicMaterial && IsValid(DynamicMaterial))
            {
                // Safe color determination
                FLinearColor EnemyColor = FLinearColor::Red; // Default for Normal
                
                if (UClass* MyClass = GetClass())
                {
                    if (IsValid(MyClass))
                    {
                        FString ClassName = MyClass->GetName();
                        
                        if (ClassName.Contains(TEXT("Heavy")))
                            EnemyColor = FLinearColor::Blue;
                        else if (ClassName.Contains(TEXT("Ranged")))
                            EnemyColor = FLinearColor::Yellow;
                        else if (ClassName.Contains(TEXT("Dash")))
                            EnemyColor = FLinearColor::Green;
                        else if (ClassName.Contains(TEXT("Aura")))
                            EnemyColor = FLinearColor(1.0f, 0.0f, 1.0f); // Purple
                        else if (ClassName.Contains(TEXT("Slime")) || ClassName.Contains(TEXT("Splitter")))
                            EnemyColor = FLinearColor(1.0f, 0.5f, 0.0f); // Orange
                        else if (ClassName.Contains(TEXT("Gold")))
                            EnemyColor = FLinearColor(1.0f, 0.84f, 0.0f); // Gold
                    }
                }
                
                // Safe parameter setting with validation
                DynamicMaterial->SetVectorParameterValue(FName("BaseColor"), EnemyColor);
                DynamicMaterial->SetScalarParameterValue(FName("Metallic"), 0.0f);
                DynamicMaterial->SetScalarParameterValue(FName("Roughness"), 0.8f);
                
                UE_LOG(LogTemp, Warning, TEXT("[COLOR] %s: Applied color safely"), *GetName());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("[COLOR] %s: Failed to create dynamic material"), *GetName());
            }
        }

        
        // FORCE VISIBILITY
        VisualMesh->SetVisibility(true);
        VisualMesh->SetHiddenInGame(false);
        
        UE_LOG(LogTemp, Warning, TEXT("[DEBUG] Enemy %s: VisualMesh configured - Mesh=%s, Visible=%s"), 
            *GetName(),
            VisualMesh->GetStaticMesh() ? *VisualMesh->GetStaticMesh()->GetName() : TEXT("NULL"),
            VisualMesh->IsVisible() ? TEXT("YES") : TEXT("NO"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[DEBUG] Enemy %s: VisualMesh is NULL!"), *GetName());
    }

    // 2. Apply difficulty scaling (would come from game state/difficulty setting)
    float DifficultyScale = 1.f; // TODO: Get from game state
    MaxHP *= DifficultyScale;
    CurrentHP = MaxHP;

    // 3. Apply 'big' modifier
    if (Mods.bBig)
    {
        // Size x1.5 for reasonable visibility (not gigantic)
        if (VisualMesh)
        {
            FVector CurrentScale = VisualMesh->GetComponentScale();
            VisualMesh->SetWorldScale3D(CurrentScale * 1.5f);
            UE_LOG(LogTemp, Warning, TEXT("[DEBUG] Enemy %s: BIG modifier applied - New scale=%s"), 
                *GetName(), *VisualMesh->GetComponentScale().ToString());
        }

        // HP/DMG x2
        MaxHP *= 2.f;
        CurrentHP = MaxHP;
        CurrentArchetype.BaseDMG *= 2.f;

        // Speed x0.5
        if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
        {
            MovementComp->MaxWalkSpeed *= 0.5f;
        }
    }

    // 4. Apply 'immovable' modifier
    if (Mods.bImmovable)
    {
        if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
        {
            MovementComp->MaxWalkSpeed = 0.f;
        }
    }

    // 5. Apply 'dissolveSeconds' timer
    if (Mods.DissolveSeconds > 0.f)
    {
        StartDissolveTimer();
    }

    // Configure aura component if enemy has aura
    if (Arch.bHasAura && AuraComponent)
    {
        AuraComponent->SetAuraProperties(Arch.AuraRadius, Arch.AuraDPS);
        AuraComponent->bAuraActive = true;
    }
    else if (AuraComponent)
    {
        AuraComponent->bAuraActive = false;
    }

    // Apply visual effects
    OnApplyVisualEffects();

    UE_LOG(LogEnemy, Log, TEXT("Applied archetype to %s: HP=%.1f, Speed=%.1f, Big=%d, Immovable=%d, Dissolve=%.1fs"), 
           *GetName(), MaxHP, GetCharacterMovement()->MaxWalkSpeed, 
           Mods.bBig ? 1 : 0, Mods.bImmovable ? 1 : 0, Mods.DissolveSeconds);
}

void AEnemyBase::StartDissolveTimer()
{
    if (CurrentModifiers.DissolveSeconds > 0.f && GetWorld())
    {
        UE_LOG(LogEnemy, Log, TEXT("%s starting dissolve timer: %.1fs"), *GetName(), CurrentModifiers.DissolveSeconds);
        
        GetWorld()->GetTimerManager().SetTimer(
            DissolveTimerHandle,
            this,
            &AEnemyBase::OnDissolveComplete,
            CurrentModifiers.DissolveSeconds,
            false
        );
    }
}

void AEnemyBase::OnDissolveComplete()
{
    UE_LOG(LogEnemy, Log, TEXT("%s dissolved (no drops)"), *GetName());
    
    // Clear timers
    if (DissolveTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(DissolveTimerHandle);
    }
    if (DashCooldownHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(DashCooldownHandle);
    }

    // Trigger dissolve effects
    OnStartDissolve();

    // Return to pool or destroy without drops
    // TODO: Integrate with pool system
    Destroy();
}

void AEnemyBase::SetupMovement()
{
    // Enable AI movement toward player
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->bOrientRotationToMovement = true;
        MovementComp->RotationRate = FRotator(0.f, 720.f, 0.f); // Fast rotation toward target
        MovementComp->MaxWalkSpeed = CurrentArchetype.BaseSpeed;
    }
}

void AEnemyBase::ChasePlayer()
{
    // SAFETY FIRST - Multiple null checks to prevent crashes
    if (!IsValid(this))
    {
        return;
    }
    
    // Only chase if not immovable
    if (CurrentModifiers.bImmovable) 
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World || !IsValid(World))
    {
        return;
    }
    
    // SAFE player finding with multiple checks
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !IsValid(PC))
    {
        return;
    }
    
    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn || !IsValid(PlayerPawn))
    {
        return;
    }
    
    // SAFE movement component check
    UCharacterMovementComponent* MyMovement = GetCharacterMovement();
    if (!MyMovement || !IsValid(MyMovement))
    {
        return;
    }
    
    // Calculate direction to player - with safe math
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector MyLocation = GetActorLocation();
    FVector DirectionToPlayer = (PlayerLocation - MyLocation);
    DirectionToPlayer.Z = 0.0f; // Keep movement on ground plane

    // Store target location for logging
    LastTargetLocation = PlayerLocation;
    
    float DistanceToPlayer = FVector::Dist2D(PlayerLocation, MyLocation);
    
    // Early exit if too close or too far (prevent weird behavior)
    if (DistanceToPlayer < 5.0f || DistanceToPlayer > 5000.0f)
    {
        return;
    }
    
    DirectionToPlayer = DirectionToPlayer.GetSafeNormal();
    
    // SIMPLE MOVEMENT - Force AddMovementInput even without Controller
    float InputScale = 1.0f;
    if (DistanceToPlayer <= 60.0f)      InputScale = 0.2f;
    else if (DistanceToPlayer <= 100.0f) InputScale = 0.5f;

    // Ensure movement mode is Walking
    if (MyMovement->MovementMode == MOVE_None)
    {
        MyMovement->SetMovementMode(MOVE_Walking);
    }

    AddMovementInput(DirectionToPlayer, InputScale, true /*bForce*/);
    
    // SAFE rotation with bounds checking
    if (!DirectionToPlayer.IsNearlyZero())
    {
        FRotator CurrentRotation = GetActorRotation();
        FRotator TargetRotation = DirectionToPlayer.Rotation();
        
        // Ensure rotations are normalized
        CurrentRotation.Normalize();
        TargetRotation.Normalize();
        
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, World->GetDeltaSeconds(), 3.0f);
        SetActorRotation(NewRotation);
    }
    
    // Fallback: if velocity stays 0, push actor a tiny step to break inertia
    const float Dt = World->GetDeltaSeconds();
    if (MyMovement && MyMovement->Velocity.Size2D() < 1.f)
    {
        const float StepSpeed = FMath::Max(80.f, MyMovement->MaxWalkSpeed);
        const FVector Step = DirectionToPlayer * StepSpeed * Dt;
        FHitResult SweepHit;
        AddActorWorldOffset(Step, true, &SweepHit);
    }

    // DEBUG: Reduced frequency logging to prevent spam and potential crashes
    static TMap<AEnemyBase*, float> LastLogTimes;
    float CurrentTime = World->GetTimeSeconds();
    float* LastLogTimePtr = LastLogTimes.Find(this);
    
    if (!LastLogTimePtr || (CurrentTime - *LastLogTimePtr > 3.0f))
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("[CHASE] %s chasing player - Distance: %.1f"), *GetName(), DistanceToPlayer);
        LastLogTimes.Add(this, CurrentTime);
    }

    // MOVEMENT POSITION LOGGING (toggleable)
    static TAutoConsoleVariable<int32> CVarEnemyMoveLog(
        TEXT("Enemy.MovementLog"),
        1,
        TEXT("Ativa (1) ou desativa (0) logs detalhados de movimento dos inimigos. Formato: [MOVE] Nome Old=(x,y,z) New=(x,y,z) Target=(x,y,z) Dist= Delta= Speed= Nominal= MaxWalk= Time=\n"),
        ECVF_Default);

    const bool bDoMoveLog = CVarEnemyMoveLog.GetValueOnGameThread() != 0;
    if (bDoMoveLog && bHasPreviousLocation)
    {
        FVector NewLocation = GetActorLocation();
        const FVector OldLocation = PreviousLocation;
        const float FrameDelta = (NewLocation - OldLocation).Size2D();
        const float DistToTargetNow = FVector::Dist2D(NewLocation, PlayerLocation);
        float CurrentSpeedComp = 0.f;
        if (UCharacterMovementComponent* Move = GetCharacterMovement())
        {
            CurrentSpeedComp = Move->Velocity.Size2D();
        }

        // Throttle high-frequency: only log if moved > 1 uu ou a cada 0.75s
        static TMap<AEnemyBase*, float> LastMoveLogTime;
        float* LastTimePtr = LastMoveLogTime.Find(this);
        const bool bMovedEnough = FrameDelta > 1.0f;
        const bool bTimeElapsed = (!LastTimePtr) || (CurrentTime - *LastTimePtr > 0.75f);
        if (bMovedEnough || bTimeElapsed)
        {
            LastMoveLogTime.Add(this, CurrentTime);
            UE_LOG(LogTemp, Warning, TEXT("[MOVE] %s Old=(%.0f,%.0f,%.0f) New=(%.0f,%.0f,%.0f) Target=(%.0f,%.0f,%.0f) Dist=%.1f Delta=%.2f Speed=%.1f Nominal=%.1f MaxWalk=%.1f Time=%.2f"),
                *GetName(),
                OldLocation.X, OldLocation.Y, OldLocation.Z,
                NewLocation.X, NewLocation.Y, NewLocation.Z,
                PlayerLocation.X, PlayerLocation.Y, PlayerLocation.Z,
                DistToTargetNow,
                FrameDelta,
                CurrentSpeedComp,
                CurrentArchetype.BaseSpeed,
                GetCharacterMovement() ? GetCharacterMovement()->MaxWalkSpeed : -1.f,
                CurrentTime);
        }
        PreviousLocation = NewLocation;
    }
    else if (bDoMoveLog && !bHasPreviousLocation)
    {
        PreviousLocation = GetActorLocation();
        bHasPreviousLocation = true;
    }
}

void AEnemyBase::HandleDashLogic(float DeltaTime)
{
    // Base implementation - DashEnemy will override this
}

float AEnemyBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    TakeDamageSimple(ActualDamage);
    return ActualDamage;
}

void AEnemyBase::TakeDamageSimple(float Damage)
{
    CurrentHP = FMath::Max(0.f, CurrentHP - Damage);
    
    UE_LOG(LogEnemy, VeryVerbose, TEXT("%s took %.1f damage, HP: %.1f/%.1f"), 
           *GetName(), Damage, CurrentHP, MaxHP);
    
    if (CurrentHP <= 0.f)
    {
        HandleDeath(this->bIsParent);
    }
}

void AEnemyBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Log all overlaps for debugging with more detail
    UE_LOG(LogTemp, Warning, TEXT("[ENEMY-OVERLAP] %s (%s) overlapped with %s (%s)"), 
        *GetName(), 
        OverlappedComp ? *OverlappedComp->GetName() : TEXT("NULL"), 
        OtherActor ? *OtherActor->GetName() : TEXT("NULL"),
        OtherComp ? *OtherComp->GetName() : TEXT("NULL"));
    
    // Damage player on overlap
    if (AMyCharacter* Player = Cast<AMyCharacter>(OtherActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("[DAMAGE-ATTEMPT] %s attempting to damage player %s"), *GetName(), *Player->GetName());
        
        // Damage cooldown to prevent spam
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastDamageTime < 1.0f) 
        {
            UE_LOG(LogTemp, Warning, TEXT("[DAMAGE] %s damage on cooldown (%.2fs remaining)"), 
                *GetName(), 1.0f - (CurrentTime - LastDamageTime));
            return;
        }
        LastDamageTime = CurrentTime;
        
        // Apply damage based on current archetype - with fallback value
        float DamageAmount = CurrentArchetype.BaseDMG > 0.0f ? CurrentArchetype.BaseDMG : 25.0f; // Fallback damage
        
        // Create proper damage event
        FPointDamageEvent DamageEvent;
        DamageEvent.Damage = DamageAmount;
        DamageEvent.HitInfo = SweepResult;
        DamageEvent.ShotDirection = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        
        UE_LOG(LogTemp, Warning, TEXT("[DAMAGE-APPLY] %s calling TakeDamage(%.1f) on player %s"), 
            *GetName(), DamageAmount, *Player->GetName());
        
        float ActualDamage = Player->TakeDamage(DamageAmount, DamageEvent, nullptr, this);
        
        UE_LOG(LogTemp, Warning, TEXT("[DAMAGE-RESULT] Player->TakeDamage returned %.1f (expected %.1f)"), 
            ActualDamage, DamageAmount);
            
        // Visual feedback - make enemy flash or something
        if (VisualMesh)
        {
            // Create or get dynamic material for flashing
            UMaterialInstanceDynamic* Mat = VisualMesh->CreateAndSetMaterialInstanceDynamic(0);
            if (Mat)
            {
                // Flash white briefly to show damage was dealt
                Mat->SetVectorParameterValue(FName("BaseColor"), FLinearColor::White);
                
                // Reset color after brief flash
                FTimerHandle FlashTimer;
                GetWorld()->GetTimerManager().SetTimer(FlashTimer, [this]() {
                    if (VisualMesh)
                    {
                        UMaterialInstanceDynamic* ResetMat = VisualMesh->CreateAndSetMaterialInstanceDynamic(0);
                        if (ResetMat)
                        {
                            // Reset to original color based on type
                            FLinearColor OriginalColor = FLinearColor::Red;
                            FString ClassName = GetClass()->GetName();
                            
                            if (ClassName.Contains(TEXT("Heavy"))) OriginalColor = FLinearColor::Blue;
                            else if (ClassName.Contains(TEXT("Ranged"))) OriginalColor = FLinearColor::Yellow;
                            else if (ClassName.Contains(TEXT("Dash"))) OriginalColor = FLinearColor::Green;
                            
                            ResetMat->SetVectorParameterValue(FName("BaseColor"), OriginalColor);
                        }
                    }
                }, 0.1f, false);
            }
        }
    }
}

void AEnemyBase::PerformanceOptimization()
{
    // Get player distance for LOD and culling
    UWorld* World = GetWorld();
    if (!World || !IsValid(World)) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !IsValid(PC)) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn || !IsValid(PlayerPawn)) return;
    
    float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    
    // DISTANCE-BASED CULLING (like old Swarm system)
    const float MaxRenderDistance = 2000.0f; // Only render within 2000 units
    const float ReducedTickDistance = 1500.0f; // Reduce tick rate beyond 1500 units
    const float MaxVisibilityDistance = 1200.0f; // Hide mesh beyond 1200 units
    
    // 1. Mesh visibility culling
    if (VisualMesh)
    {
        bool bShouldBeVisible = DistanceToPlayer <= MaxVisibilityDistance;
        if (VisualMesh->IsVisible() != bShouldBeVisible)
        {
            VisualMesh->SetVisibility(bShouldBeVisible);
        }
    }
    
    // 2. Tick optimization - reduce tick frequency for distant enemies
    if (DistanceToPlayer > ReducedTickDistance)
    {
        // Tick every 0.2 seconds instead of every frame for distant enemies
        PrimaryActorTick.TickInterval = 0.2f;
    }
    else
    {
        // Normal tick rate for nearby enemies
        PrimaryActorTick.TickInterval = 0.0f;
    }
    
    // 3. Collision optimization - disable collision for very distant enemies
    if (DistanceToPlayer > MaxRenderDistance)
    {
        if (UCapsuleComponent* Capsule = GetCapsuleComponent())
        {
            Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
        // Não desabilitar Tick completamente para manter logs de diagnóstico; apenas reduzir frequência
        PrimaryActorTick.TickInterval = 0.5f;
    }
    else
    {
        if (UCapsuleComponent* Capsule = GetCapsuleComponent())
        {
            Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
        if (!IsActorTickEnabled())
        {
            SetActorTickEnabled(true);
        }
    }
}
