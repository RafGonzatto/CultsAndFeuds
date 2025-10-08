#include "World/XPDropService.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"

void UXPDropService::SpawnXPDrop_Implementation(int32 XPAmount, const FVector& Location)
{
    if (UWorld* World = GetWorld())
    {
        if (UClass* ActorClass = XPDropActorClass.IsValid() ? XPDropActorClass.Get() : XPDropActorClass.LoadSynchronous())
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            const FVector SpawnLocation = Location + FVector(0.f, 0.f, SpawnHeightOffset);
            if (AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams))
            {
                if (FProperty* XPProperty = SpawnedActor->GetClass()->FindPropertyByName(TEXT("XPAmount")))
                {
                    if (FIntProperty* XPIntProperty = CastField<FIntProperty>(XPProperty))
                    {
                        XPIntProperty->SetPropertyValue_InContainer(SpawnedActor, FMath::Max(1, XPAmount));
                    }
                }
            }
        }
    }

    OnSpawnXPDrop(XPAmount, Location);
}
