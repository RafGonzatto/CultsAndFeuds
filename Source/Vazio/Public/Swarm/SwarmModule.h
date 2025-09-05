#pragma once
#include "Modules/ModuleManager.h"

class FSwarmModule final : public IModuleInterface {
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
