#include "Systems/EnemyImplementationCVars.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<int32> CVarEnemyImpl(
    TEXT("enemy.impl"), /*0=Actor,1=Mass*/
    1, TEXT("Select enemy implementation: 0=Actor, 1=Mass"),
    ECVF_Default);
int32 GetEnemyImpl() { return CVarEnemyImpl.GetValueOnGameThread(); }
