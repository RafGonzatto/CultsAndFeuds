#include "UI/Widgets/HUDSelectMapWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Swarm/SwarmGameFlow.h"
#include "Multiplayer/SteamMultiplayerSubsystem.h"

UHUDSelectMapWidget::UHUDSelectMapWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    CurrentSearchText = TEXT("");
    CurrentFilterType = TEXT("All");
    InitializeDefaultMaps();
}

TSharedRef<SWidget> UHUDSelectMapWidget::BuildBody()
{
    UpdateFilteredMaps();

    return SNew(SBorder)
        .BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
        .Padding(0)
        [
            SNew(SBox)
            .WidthOverride(1200.0f)
            .HeightOverride(800.0f)
            [
                SNew(SBorder)
                .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                .BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.06f, 1.0f)) // #0d0d0f
                .Padding(FMargin(24.0f))
                [
                    SNew(SHorizontalBox)
                    
                    // Left Column - Map Browser
                    + SHorizontalBox::Slot()
                    .FillWidth(0.3f)
                    .Padding(0, 0, 12, 0)
                    [
                        SNew(SBorder)
                        .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                        .BorderBackgroundColor(FLinearColor(0.07f, 0.07f, 0.09f, 1.0f)) // #121216
                        .Padding(FMargin(16.0f))
                        [
                            SNew(SVerticalBox)
                            
                            // Search Box
                            + SVerticalBox::Slot()
                            .AutoHeight()
                            .Padding(0, 0, 0, 12)
                            [
                                SAssignNew(SearchBox, SEditableTextBox)
                                .HintText(FText::FromString(TEXT("Search maps...")))
                                .OnTextChanged_Lambda([this](const FText& Text)
                                {
                                    CurrentSearchText = Text.ToString();
                                    UpdateFilteredMaps();
                                })
                            ]
                            
                            // Filter Chips
                            + SVerticalBox::Slot()
                            .AutoHeight()
                            .Padding(0, 0, 0, 16)
                            [
                                SAssignNew(FilterChipsContainer, SHorizontalBox)
                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .Padding(0, 0, 4, 0)
                                [
                                    CreateFilterChip(TEXT("All"))
                                ]
                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .Padding(4, 0, 4, 0)
                                [
                                    CreateFilterChip(TEXT("Urban"))
                                ]
                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .Padding(4, 0, 4, 0)
                                [
                                    CreateFilterChip(TEXT("Desert"))
                                ]
                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .Padding(4, 0, 0, 0)
                                [
                                    CreateFilterChip(TEXT("Forest"))
                                ]
                            ]
                            
                            // Map List
                            + SVerticalBox::Slot()
                            .FillHeight(1.0f)
                            [
                                SNew(SScrollBox)
                                + SScrollBox::Slot()
                                [
                                    SAssignNew(MapListContainer, SVerticalBox)
                                ]
                            ]
                        ]
                    ]
                    
                    // Center Column - Map Preview & Details
                    + SHorizontalBox::Slot()
                    .FillWidth(0.4f)
                    .Padding(12, 0, 12, 0)
                    [
                        SNew(SBorder)
                        .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                        .BorderBackgroundColor(FLinearColor(0.07f, 0.07f, 0.09f, 1.0f))
                        .Padding(FMargin(16.0f))
                        [
                            SNew(SVerticalBox)
                            
                            // Selected Map Title
                            + SVerticalBox::Slot()
                            .AutoHeight()
                            .Padding(0, 0, 0, 12)
                            [
                                SAssignNew(SelectedMapTitle, STextBlock)
                                .Text(FText::FromString(TEXT("Select a Map")))
                                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
                                .ColorAndOpacity(FLinearColor::White)
                                .Justification(ETextJustify::Center)
                            ]
                            
                            // Map Preview (Placeholder)
                            + SVerticalBox::Slot()
                            .FillHeight(0.6f)
                            .Padding(0, 0, 0, 12)
                            [
                                SNew(SBorder)
                                .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                                .BorderBackgroundColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f))
                                .HAlign(HAlign_Center)
                                .VAlign(VAlign_Center)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString(TEXT("Map Preview")))
                                    .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
                                ]
                            ]
                            
                            // Map Details
                            + SVerticalBox::Slot()
                            .FillHeight(0.4f)
                            [
                                SAssignNew(SelectedMapDetails, STextBlock)
                                .Text(FText::FromString(TEXT("Select a map to see details")))
                                .ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
                                .AutoWrapText(true)
                            ]
                        ]
                    ]
                    
                    // Right Column - Steam Friends & Session Settings
                    + SHorizontalBox::Slot()
                    .FillWidth(0.3f)
                    .Padding(12, 0, 0, 0)
                    [
                        SNew(SBorder)
                        .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                        .BorderBackgroundColor(FLinearColor(0.07f, 0.07f, 0.09f, 1.0f))
                        .Padding(FMargin(16.0f))
                        [
                            SNew(SVerticalBox)
                            
                            // Steam Authentication Section
                            + SVerticalBox::Slot()
                            .AutoHeight()
                            .Padding(0, 0, 0, 16)
                            [
                                SNew(SVerticalBox)
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 0, 0, 8)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString(TEXT("Steam Account")))
                                    .Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
                                    .ColorAndOpacity(FLinearColor::White)
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 0, 0, 4)
                                [
                                    SAssignNew(SteamStatusText, STextBlock)
                                    .Text(FText::FromString(TEXT("Not Connected")))
                                    .Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
                                    .ColorAndOpacity(FLinearColor::Red)
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 0, 0, 8)
                                [
                                    SNew(SButton)
                                    .Text(FText::FromString(TEXT("Connect Steam")))
                                    .OnClicked_Lambda([this]()
                                    {
                                        InitializeSteamAuth();
                                        return FReply::Handled();
                                    })
                                ]
                            ]
                            
                            // Difficulty Selection
                            + SVerticalBox::Slot()
                            .AutoHeight()
                            .Padding(0, 0, 0, 16)
                            [
                                SNew(SVerticalBox)
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 0, 0, 8)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString(TEXT("Difficulty")))
                                    .Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
                                    .ColorAndOpacity(FLinearColor::White)
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                [
                                    SNew(SHorizontalBox)
                                    + SHorizontalBox::Slot()
                                    .AutoWidth()
                                    .Padding(0, 0, 4, 0)
                                    [
                                        SNew(SButton)
                                        .Text(FText::FromString(TEXT("Easy")))
                                        .ButtonColorAndOpacity(FLinearColor::Green)
                                    ]
                                    + SHorizontalBox::Slot()
                                    .AutoWidth()
                                    .Padding(4, 0, 4, 0)
                                    [
                                        SNew(SButton)
                                        .Text(FText::FromString(TEXT("Normal")))
                                        .ButtonColorAndOpacity(FLinearColor::Yellow)
                                    ]
                                    + SHorizontalBox::Slot()
                                    .AutoWidth()
                                    .Padding(4, 0, 0, 0)
                                    [
                                        SNew(SButton)
                                        .Text(FText::FromString(TEXT("Hard")))
                                        .ButtonColorAndOpacity(FLinearColor::Red)
                                    ]
                                ]
                            ]
                            
                            // Friends List
                            + SVerticalBox::Slot()
                            .FillHeight(1.0f)
                            .Padding(0, 0, 0, 16)
                            [
                                SNew(SVerticalBox)
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 0, 0, 8)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString(TEXT("Online Friends")))
                                    .Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
                                    .ColorAndOpacity(FLinearColor::White)
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(1.0f)
                                [
                                    SNew(SScrollBox)
                                    + SScrollBox::Slot()
                                    [
                                        SAssignNew(FriendsListContainer, SVerticalBox)
                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        [
                                            SNew(STextBlock)
                                            .Text(FText::FromString(TEXT("Connect Steam to see friends")))
                                            .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
                                        ]
                                    ]
                                ]
                            ]
                            
                            // Start Session Button
                            + SVerticalBox::Slot()
                            .AutoHeight()
                            [
                                SAssignNew(ConfirmButton, SButton)
                                .Text(FText::FromString(TEXT("Start Session")))
                                .HAlign(HAlign_Center)
                                .OnClicked_Lambda([this]()
                                {
                                    OnConfirmSelection();
                                    return FReply::Handled();
                                })
                                .IsEnabled_Lambda([this]()
                                {
                                    return !SelectedMap.MapName.IsEmpty();
                                })
                            ]
                        ]
                    ]
                ]
            ]
        ];
}

void UHUDSelectMapWidget::InitializeDefaultMaps()
{
    AvailableMaps.Empty();

    // Default Battle_Main map
    FMapData BattleMain;
    BattleMain.MapName = TEXT("Battle_Main");
    BattleMain.MapDisplayName = TEXT("Battle Arena");
    BattleMain.MapType = TEXT("Urban");
    BattleMain.Difficulty = TEXT("Normal");
    AvailableMaps.Add(BattleMain);

    // Add some placeholder maps
    FMapData DesertMap;
    DesertMap.MapName = TEXT("Desert_Arena");
    DesertMap.MapDisplayName = TEXT("Desert Combat");
    DesertMap.MapType = TEXT("Desert");
    DesertMap.Difficulty = TEXT("Hard");
    AvailableMaps.Add(DesertMap);

    FMapData ForestMap;
    ForestMap.MapName = TEXT("Forest_Arena");
    ForestMap.MapDisplayName = TEXT("Forest Battleground");
    ForestMap.MapType = TEXT("Forest");
    ForestMap.Difficulty = TEXT("Easy");
    AvailableMaps.Add(ForestMap);

    // Set default selection
    if (AvailableMaps.Num() > 0)
    {
        SelectedMap = AvailableMaps[0];
    }
}

void UHUDSelectMapWidget::UpdateFilteredMaps()
{
    FilteredMaps.Empty();

    for (const FMapData& Map : AvailableMaps)
    {
        bool bPassesFilter = true;

        // Search filter
        if (!CurrentSearchText.IsEmpty())
        {
            bPassesFilter = Map.MapDisplayName.Contains(CurrentSearchText) || 
                          Map.MapType.Contains(CurrentSearchText);
        }

        // Type filter
        if (bPassesFilter && CurrentFilterType != TEXT("All"))
        {
            bPassesFilter = Map.MapType == CurrentFilterType;
        }

        if (bPassesFilter)
        {
            FilteredMaps.Add(Map);
        }
    }

    // Rebuild map list UI
    if (MapListContainer.IsValid())
    {
        MapListContainer->ClearChildren();
        
        for (const FMapData& Map : FilteredMaps)
        {
            MapListContainer->AddSlot()
            .AutoHeight()
            .Padding(0, 0, 0, 8)
            [
                CreateMapListItem(Map)
            ];
        }
    }
}

TSharedRef<SWidget> UHUDSelectMapWidget::CreateMapListItem(const FMapData& MapData)
{
    return SNew(SButton)
        .ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button"))
        .OnClicked_Lambda([this, MapData]()
        {
            UpdateMapSelection(MapData);
            return FReply::Handled();
        })
        [
            SNew(SBorder)
            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.12f, 1.0f))
            .Padding(FMargin(12.0f))
            [
                SNew(SHorizontalBox)
                
                // Thumbnail placeholder
                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                .Padding(0, 0, 12, 0)
                [
                    SNew(SBox)
                    .WidthOverride(64.0f)
                    .HeightOverride(64.0f)
                    [
                        SNew(SBorder)
                        .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                        .BorderBackgroundColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f))
                        .HAlign(HAlign_Center)
                        .VAlign(VAlign_Center)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString(MapData.MapType.Left(1)))
                            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
                            .ColorAndOpacity(FLinearColor::White)
                        ]
                    ]
                ]
                
                // Map info
                + SHorizontalBox::Slot()
                .FillWidth(1.0f)
                .VAlign(VAlign_Center)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(MapData.MapDisplayName))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
                        .ColorAndOpacity(FLinearColor::White)
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(FString::Printf(TEXT("%s - %s"), *MapData.MapType, *MapData.Difficulty)))
                        .Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
                        .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
                    ]
                ]
            ]
        ];
}

TSharedRef<SWidget> UHUDSelectMapWidget::CreateFilterChip(const FString& FilterType)
{
    bool bIsActive = (CurrentFilterType == FilterType);
    
    return SNew(SButton)
        .ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button"))
        .ButtonColorAndOpacity(bIsActive ? FLinearColor(0.2f, 0.4f, 0.8f, 1.0f) : FLinearColor(0.15f, 0.15f, 0.15f, 1.0f))
        .OnClicked_Lambda([this, FilterType]()
        {
            CurrentFilterType = FilterType;
            UpdateFilteredMaps();
            return FReply::Handled();
        })
        [
            SNew(STextBlock)
            .Text(FText::FromString(FilterType))
            .Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
            .ColorAndOpacity(FLinearColor::White)
        ];
}

void UHUDSelectMapWidget::UpdateMapSelection(const FMapData& MapData)
{
    SelectedMap = MapData;
    
    if (SelectedMapTitle.IsValid())
    {
        SelectedMapTitle->SetText(FText::FromString(MapData.MapDisplayName));
    }
    
    if (SelectedMapDetails.IsValid())
    {
        FString Details = FString::Printf(TEXT("Map Type: %s\nDifficulty: %s\nLevel: %s\n\nA challenging arena where players battle against swarms of enemies. Test your skills and upgrade your abilities to survive increasingly difficult waves."), 
            *MapData.MapType, *MapData.Difficulty, *MapData.MapName);
        SelectedMapDetails->SetText(FText::FromString(Details));
    }
}

void UHUDSelectMapWidget::OnConfirmSelection()
{
    if (SelectedMap.MapName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[HUDSelectMap] No map selected"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[HUDSelectMap] Starting session with map: %s"), *SelectedMap.MapName);

    // Create multiplayer session if Steam is initialized
    if (UGameInstance* GI = GetGameInstance())
    {
        if (USteamMultiplayerSubsystem* SteamSubsystem = GI->GetSubsystem<USteamMultiplayerSubsystem>())
        {
            if (SteamSubsystem->IsSteamInitialized())
            {
                FString SessionName = FString::Printf(TEXT("%s_%s_Session"), *SelectedMap.MapDisplayName, *SteamSubsystem->GetPlayerSteamName());
                SteamSubsystem->CreateMultiplayerSession(SessionName, 4); // Max 4 players
            }
        }

        // Set up return point similar to SwarmArenaModalWidget
        if (UWorld* World = GetWorld())
        {
            if (USwarmGameFlow* Flow = GI->GetSubsystem<USwarmGameFlow>())
            {
                // Find return marker
                FTransform ReturnXf;
                bool bFound = false;
                TArray<AActor*> AllActors;
                UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
                for (AActor* A : AllActors)
                {
                    if (A && (A->ActorHasTag(TEXT("ArenaReturn")) || A->GetName().Contains(TEXT("ArenaReturn"))))
                    {
                        ReturnXf = A->GetActorTransform();
                        bFound = true;
                        break;
                    }
                }
                
                if (!bFound)
                {
                    if (APlayerController* PC = World->GetFirstPlayerController())
                    {
                        if (APawn* P = PC->GetPawn())
                        {
                            ReturnXf = P->GetActorTransform();
                        }
                    }
                }

                const FName CurrentLevelName = FName(*UGameplayStatics::GetCurrentLevelName(World, true));
                Flow->SetReturnPoint(CurrentLevelName, ReturnXf);
                Flow->EnterArena(*SelectedMap.MapName);
            }
        }
    }

    // Broadcast selection event
    OnMapSelected.Broadcast(SelectedMap);
    
    // Close modal
    HandleClose();
}

void UHUDSelectMapWidget::OnSettingsClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("[HUDSelectMap] Settings clicked - placeholder"));
    // TODO: Open settings modal
}

void UHUDSelectMapWidget::OnHelpClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("[HUDSelectMap] Help clicked - placeholder"));
    // TODO: Open help modal
}

void UHUDSelectMapWidget::InitializeSteamAuth()
{
    UE_LOG(LogTemp, Warning, TEXT("[HUDSelectMap] InitializeSteamAuth"));
    
    if (UGameInstance* GI = GetGameInstance())
    {
        if (USteamMultiplayerSubsystem* SteamSubsystem = GI->GetSubsystem<USteamMultiplayerSubsystem>())
        {
            if (!SteamSubsystem->IsSteamInitialized())
            {
                // Bind to Steam auth completion event
                SteamSubsystem->OnSteamAuthComplete.AddDynamic(this, &UHUDSelectMapWidget::OnSteamAuthCompleted);
                SteamSubsystem->OnFriendsListUpdated.AddDynamic(this, &UHUDSelectMapWidget::OnSteamFriendsUpdated);
                
                SteamSubsystem->InitializeSteam();
            }
            else
            {
                // Already initialized, just refresh friends list
                SteamSubsystem->RefreshFriendsList();
            }
        }
    }
}

TArray<FString> UHUDSelectMapWidget::GetSteamFriends()
{
    TArray<FString> FriendNames;
    
    if (UGameInstance* GI = GetGameInstance())
    {
        if (USteamMultiplayerSubsystem* SteamSubsystem = GI->GetSubsystem<USteamMultiplayerSubsystem>())
        {
            TArray<FSteamFriend> OnlineFriends = SteamSubsystem->GetOnlineFriends();
            for (const FSteamFriend& Friend : OnlineFriends)
            {
                FriendNames.Add(Friend.DisplayName);
            }
        }
    }
    
    return FriendNames;
}

void UHUDSelectMapWidget::InviteFriend(const FString& FriendName)
{
    UE_LOG(LogTemp, Warning, TEXT("[HUDSelectMap] Inviting friend: %s"), *FriendName);
    
    if (UGameInstance* GI = GetGameInstance())
    {
        if (USteamMultiplayerSubsystem* SteamSubsystem = GI->GetSubsystem<USteamMultiplayerSubsystem>())
        {
            // Find friend's Steam ID by name
            TArray<FSteamFriend> Friends = SteamSubsystem->GetFriendsList();
            for (const FSteamFriend& Friend : Friends)
            {
                if (Friend.DisplayName == FriendName)
                {
                    SteamSubsystem->InviteFriendToSession(Friend.SteamID);
                    break;
                }
            }
        }
    }
}

void UHUDSelectMapWidget::OnSteamAuthCompleted(bool bSuccess)
{
    UE_LOG(LogTemp, Warning, TEXT("[HUDSelectMap] Steam auth completed: %s"), bSuccess ? TEXT("Success") : TEXT("Failed"));
    
    if (SteamStatusText.IsValid())
    {
        if (bSuccess)
        {
            if (UGameInstance* GI = GetGameInstance())
            {
                if (USteamMultiplayerSubsystem* SteamSubsystem = GI->GetSubsystem<USteamMultiplayerSubsystem>())
                {
                    FString StatusText = FString::Printf(TEXT("Connected as %s"), *SteamSubsystem->GetPlayerSteamName());
                    SteamStatusText->SetText(FText::FromString(StatusText));
                    SteamStatusText->SetColorAndOpacity(FLinearColor::Green);
                }
            }
        }
        else
        {
            SteamStatusText->SetText(FText::FromString(TEXT("Connection Failed")));
            SteamStatusText->SetColorAndOpacity(FLinearColor::Red);
        }
    }
}

void UHUDSelectMapWidget::OnSteamFriendsUpdated(const TArray<FSteamFriend>& Friends)
{
    UE_LOG(LogTemp, Warning, TEXT("[HUDSelectMap] Steam friends list updated: %d friends"), Friends.Num());
    
    // Update local friends cache for easier access
    SteamFriends.Empty();
    for (const FSteamFriend& Friend : Friends)
    {
        if (Friend.bIsOnline)
        {
            SteamFriends.Add(Friend.DisplayName);
        }
    }
    
    // Update the friends list UI
    UpdateFriendsListUI();
    
    UE_LOG(LogTemp, Warning, TEXT("[HUDSelectMap] %d online friends cached"), SteamFriends.Num());
}

void UHUDSelectMapWidget::UpdateFriendsListUI()
{
    if (!FriendsListContainer.IsValid())
    {
        return;
    }

    FriendsListContainer->ClearChildren();

    if (UGameInstance* GI = GetGameInstance())
    {
        if (USteamMultiplayerSubsystem* SteamSubsystem = GI->GetSubsystem<USteamMultiplayerSubsystem>())
        {
            if (SteamSubsystem->IsSteamInitialized())
            {
                TArray<FSteamFriend> OnlineFriends = SteamSubsystem->GetOnlineFriends();
                
                if (OnlineFriends.Num() > 0)
                {
                    for (const FSteamFriend& Friend : OnlineFriends)
                    {
                        FriendsListContainer->AddSlot()
                        .AutoHeight()
                        .Padding(0, 0, 0, 4)
                        [
                            SNew(SBorder)
                            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                            .BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.12f, 1.0f))
                            .Padding(FMargin(8.0f))
                            [
                                SNew(SHorizontalBox)
                                + SHorizontalBox::Slot()
                                .FillWidth(1.0f)
                                .VAlign(VAlign_Center)
                                [
                                    SNew(SVerticalBox)
                                    + SVerticalBox::Slot()
                                    .AutoHeight()
                                    [
                                        SNew(STextBlock)
                                        .Text(FText::FromString(Friend.DisplayName))
                                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
                                        .ColorAndOpacity(FLinearColor::White)
                                    ]
                                    + SVerticalBox::Slot()
                                    .AutoHeight()
                                    [
                                        SNew(STextBlock)
                                        .Text(FText::FromString(Friend.bIsInGame ? TEXT("In Game") : TEXT("Online")))
                                        .Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
                                        .ColorAndOpacity(Friend.bIsInGame ? FLinearColor::Yellow : FLinearColor::Green)
                                    ]
                                ]
                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .VAlign(VAlign_Center)
                                [
                                    SNew(SButton)
                                    .Text(FText::FromString(TEXT("Invite")))
                                    .OnClicked_Lambda([this, Friend]()
                                    {
                                        InviteFriend(Friend.DisplayName);
                                        return FReply::Handled();
                                    })
                                ]
                            ]
                        ];
                    }
                }
                else
                {
                    FriendsListContainer->AddSlot()
                    .AutoHeight()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("No friends online")))
                        .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
                    ];
                }
            }
            else
            {
                FriendsListContainer->AddSlot()
                .AutoHeight()
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("Connect Steam to see friends")))
                    .ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
                ];
            }
        }
    }
}

// Implementation of missing functions
void UHUDSelectMapWidget::OnSearchTextChanged(const FText& Text)
{
    CurrentSearchText = Text.ToString();
    RefreshMapList();
}

void UHUDSelectMapWidget::OnMapTypeFilterClicked(const FString& MapType)
{
    CurrentFilterType = MapType;
    RefreshMapList();
}

void UHUDSelectMapWidget::OnMapItemClicked(const FMapData& MapData)
{
    SelectedMap = MapData;
    // Update UI to show selection
    RefreshMapList();
}

void UHUDSelectMapWidget::RefreshMapList()
{
    // Update filtered maps based on current search and filter
    UpdateFilteredMaps();
    
    // Update the UI display (this would rebuild the map list widget)
    // For now, we'll just ensure the map data is updated
    if (SelectedMapTitle.IsValid())
    {
        SelectedMapTitle->SetText(FText::FromString(SelectedMap.MapName));
    }
    
    if (SelectedMapDetails.IsValid())
    {
        FString Details = FString::Printf(TEXT("Type: %s | Players: %d-%d"), 
            *SelectedMap.MapType, SelectedMap.MinPlayers, SelectedMap.MaxPlayers);
        SelectedMapDetails->SetText(FText::FromString(Details));
    }
}
