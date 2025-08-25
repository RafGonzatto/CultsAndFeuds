#include "World/Common/Effects/ClickArrowIndicator.h"
#include "Components/ArrowComponent.h"

AClickArrowIndicator::AClickArrowIndicator()
{
	PrimaryActorTick.bCanEverTick = false;

	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	SetRootComponent(Arrow);

	Arrow->SetArrowColor(FLinearColor(1.f, 1.f, 0.f, 1.f)); // amarelo
	Arrow->ArrowSize = 2.0f; // maior
	Arrow->bTreatAsASprite = false;
	Arrow->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
}