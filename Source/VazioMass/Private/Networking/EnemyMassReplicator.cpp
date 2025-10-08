#include "Networking/EnemyMassReplicator.h"
#include "Net/UnrealNetwork.h"

AEnemyMassReplicator::AEnemyMassReplicator()
{
    bReplicates = true;
    SetReplicateMovement(false);
    NetUpdateFrequency = 15.0f; // tune as needed
    MinNetUpdateFrequency = 10.0f;
}

void AEnemyMassReplicator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AEnemyMassReplicator, NetStates);
}
