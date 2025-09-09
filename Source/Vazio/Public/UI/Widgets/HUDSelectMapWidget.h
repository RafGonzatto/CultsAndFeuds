#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/BaseModalWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Components/ScrollBox.h"
#include "HUDSelectMapWidget.generated.h"

// Forward declarations
struct FSteamFriend;

// Map data structure
USTRUCT(BlueprintType)
struct FMapData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    FString MapName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    FString MapDisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    FString MapType; // Urban, Desert, Forest, Snow

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    FString Difficulty; // Easy, Normal, Hard

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    int32 MinPlayers = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    int32 MaxPlayers = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    class UTexture2D* Thumbnail;

    FMapData()
    {
        MapName = TEXT("Battle_Main");
        MapDisplayName = TEXT("Battle Arena");
        MapType = TEXT("Urban");
        Difficulty = TEXT("Normal");
        Thumbnail = nullptr;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMapSelected, const FMapData&, SelectedMap);

UCLASS()
class VAZIO_API UHUDSelectMapWidget : public UBaseModalWidget
{
    GENERATED_BODY()

public:
    UHUDSelectMapWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UPROPERTY(BlueprintAssignable, Category = "Map Selection")
    FOnMapSelected OnMapSelected;

    // Initialize Steam authentication (placeholder)
    UFUNCTION(BlueprintCallable, Category = "Steam")
    void InitializeSteamAuth();

    // Get Steam friends list (placeholder)
    UFUNCTION(BlueprintCallable, Category = "Steam")
    TArray<FString> GetSteamFriends();

    // Invite friend to session (placeholder)
    UFUNCTION(BlueprintCallable, Category = "Steam")
    void InviteFriend(const FString& FriendName);

    // Steam callbacks
    UFUNCTION()
    void OnSteamAuthCompleted(bool bSuccess);

    UFUNCTION()
    void OnSteamFriendsUpdated(const TArray<struct FSteamFriend>& Friends);

protected:
    virtual FText GetTitle() const override { return FText::FromString(TEXT("Select Map")); }
    virtual TSharedRef<SWidget> BuildBody() override;

    // Map filtering and search
    UFUNCTION()
    void OnSearchTextChanged(const FText& Text);

    UFUNCTION()
    void OnMapTypeFilterClicked(const FString& MapType);

    UFUNCTION()
    void OnMapItemClicked(const FMapData& MapData);

    UFUNCTION()
    void OnConfirmSelection();

    UFUNCTION()
    void OnSettingsClicked();

    UFUNCTION()
    void OnHelpClicked();

private:
    // Available maps
    UPROPERTY()
    TArray<FMapData> AvailableMaps;

    // Filtered maps based on search/filter
    UPROPERTY()
    TArray<FMapData> FilteredMaps;

    // Currently selected map
    UPROPERTY()
    FMapData SelectedMap;

    // Current search text
    FString CurrentSearchText;

    // Current filter type
    FString CurrentFilterType;

    // UI References
    TSharedPtr<class SEditableTextBox> SearchBox;
    TSharedPtr<class SVerticalBox> MapListContainer;
    TSharedPtr<class SHorizontalBox> FilterChipsContainer;
    TSharedPtr<class STextBlock> SelectedMapTitle;
    TSharedPtr<class STextBlock> SelectedMapDetails;
    TSharedPtr<class SButton> ConfirmButton;
    TSharedPtr<class SVerticalBox> FriendsListContainer;
    TSharedPtr<class STextBlock> SteamStatusText;

    // Steam friends (placeholder)
    TArray<FString> SteamFriends;

    // Initialize default maps
    void InitializeDefaultMaps();

    // Update filtered maps based on search and filter
    void UpdateFilteredMaps();

    // Create map list item widget
    TSharedRef<SWidget> CreateMapListItem(const FMapData& MapData);

    // Create filter chip widget
    TSharedRef<SWidget> CreateFilterChip(const FString& FilterType);

    // Update map selection display
    void UpdateMapSelection(const FMapData& MapData);
    
    // Refresh the map list display
    void RefreshMapList();

    // Update friends list UI
    void UpdateFriendsListUI();
};
