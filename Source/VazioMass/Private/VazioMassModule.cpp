#include "VazioMassModule.h"
#include "Modules/ModuleManager.h"
#include "Debug/MassProfilingCommands.h"

IMPLEMENT_MODULE(FVazioMass, VazioMass);

void FVazioMass::StartupModule()
{
	FMassProfilingCommands::Register();
}

void FVazioMass::ShutdownModule()
{
	FMassProfilingCommands::Unregister();
}
