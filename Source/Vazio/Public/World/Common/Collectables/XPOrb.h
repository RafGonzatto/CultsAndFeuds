#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "XPOrb.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class VAZIO_API AXPOrb : public AActor {
    GENERATED_BODY()
public:
    AXPOrb();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components") USphereComponent* Sphere;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components") UStaticMeshComponent* Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XP") int32 XPAmount = 10;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement") float AttractionRadius = 300.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement") float MoveSpeed = 500.f;

protected:
    UPROPERTY() AActor* TargetPlayer = nullptr;

    UFUNCTION()
    void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                   int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
