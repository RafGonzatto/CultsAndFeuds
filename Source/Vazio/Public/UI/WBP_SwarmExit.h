#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WBP_SwarmExit.generated.h"

UCLASS()
class VAZIO_API UWBP_SwarmExit : public UUserWidget {
  GENERATED_BODY()
public:
  UFUNCTION(BlueprintCallable, Category="Swarm|UI") void OnExitClicked();
};

