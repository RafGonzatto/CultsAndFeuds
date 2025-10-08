#include "Logging/VazioLogFacade.h"
#include "Modules/ModuleManager.h"

class FVazioSharedModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        FVazioLog::FVazioLogFacade::Initialize();
    }

    virtual void ShutdownModule() override
    {
        FVazioLog::FVazioLogFacade::Shutdown();
    }
};

IMPLEMENT_MODULE(FVazioSharedModule, VazioShared);
