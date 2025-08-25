#include "World/Common/GameModes/CityGameMode.h"
#include "World/Common/Player/MyCharacter.h"
#include "World/Common/Player/MyPlayerController.h"

ACityGameMode::ACityGameMode()
{
	DefaultPawnClass = AMyCharacter::StaticClass();
	PlayerControllerClass = AMyPlayerController::StaticClass();
}
