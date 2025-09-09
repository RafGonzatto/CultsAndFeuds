#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class VAZIO_API SLevelText : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLevelText) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void UpdateLevel(int32 NewLevel);

private:
    TSharedPtr<class STextBlock> LevelTextBlock;
    int32 CurrentLevel = 1;
    
    // Text binding method
    FText GetLevelText() const;
};
