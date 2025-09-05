#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/BaseModalWidget.h"
#include "SwarmArenaModalWidget.generated.h"

UCLASS()
class VAZIO_API USwarmArenaModalWidget : public UBaseModalWidget
{
    GENERATED_BODY()
protected:
    virtual FText GetTitle() const override { return FText::FromString(TEXT("Arena")); }
    virtual TSharedRef<SWidget> BuildBody() override;
};

