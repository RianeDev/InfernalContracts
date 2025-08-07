// HandManager.cpp - Complete Implementation
#include "HandManager.h"
#include "CardActor.h"
#include "Engine/World.h"
#include "Components/PanelWidget.h"
#include "Blueprint/UserWidget.h"
#include "ICardWidget.h"
#include "Engine/Engine.h"

AHandManager::AHandManager()
{
    PrimaryActorTick.bCanEverTick = false;
    MaxHandSize = 7;
    StartingHandSize = 5;
}

void AHandManager::BeginPlay()
{
    Super::BeginPlay();
}

// ==== CORE HAND MANAGEMENT ====

bool AHandManager::AddCardToHand(int32 CardID)
{
    if (CurrentHand.Num() >= MaxHandSize)
    {
        UE_LOG(LogTemp, Warning, TEXT("[KCK HandManager] Hand is full! Cannot add card ID %d"), CardID);
        return false;
    }

    FCardData* FoundCard = FindCardByID(CardID);
    if (!FoundCard)
    {
        UE_LOG(LogTemp, Warning, TEXT("[KCK HandManager] Card ID %d not found in CardDataTable"), CardID);
        return false;
    }

    // Add to hand
    CurrentHand.Add(*FoundCard);
    int32 NewIndex = CurrentHand.Num() - 1;

    // Create UI widget for the card
    CreateCardWidget(*FoundCard, NewIndex);

    // Broadcast event
    OnHandUpdated.Broadcast(CurrentHand, CurrentHand.Num());

    UE_LOG(LogTemp, Log, TEXT("[KCK HandManager] Added card '%s' to hand (Index: %d)"),
        *FoundCard->Name.ToString(), NewIndex);

    return true;
}

bool AHandManager::RemoveCardFromHand(int32 HandIndex)
{
    if (!CurrentHand.IsValidIndex(HandIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("[KCK HandManager] Invalid hand index: %d"), HandIndex);
        return false;
    }

    FCardData RemovedCard = CurrentHand[HandIndex];
    CurrentHand.RemoveAt(HandIndex);

    // Destroy the UI widget
    DestroyCardWidget(HandIndex);

    // Refresh all widgets after this index (their indices shifted)
    RefreshHandUI();

    OnHandUpdated.Broadcast(CurrentHand, CurrentHand.Num());

    UE_LOG(LogTemp, Log, TEXT("[KCK HandManager] Removed card '%s' from hand"),
        *RemovedCard.Name.ToString());

    return true;
}

void AHandManager::ClearHand()
{
    CurrentHand.Empty();

    // Clear all UI widgets
    for (UUserWidget* Widget : HandWidgets)
    {
        if (Widget)
        {
            Widget->RemoveFromParent();
        }
    }
    HandWidgets.Empty();

    OnHandUpdated.Broadcast(CurrentHand, 0);
    UE_LOG(LogTemp, Log, TEXT("[KCK HandManager] Hand cleared"));
}

void AHandManager::DrawStartingHand()
{
    ClearHand();
    DrawCards(StartingHandSize);
}

void AHandManager::DrawCards(int32 Count)
{
    for (int32 i = 0; i < Count; i++)
    {
        if (PlayerDeck.Num() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("[KCK HandManager] Cannot draw card - deck is empty"));
            break;
        }

        if (CurrentHand.Num() >= MaxHandSize)
        {
            UE_LOG(LogTemp, Warning, TEXT("[KCK HandManager] Cannot draw card - hand is full"));
            break;
        }

        // Draw top card from deck
        FCardData DrawnCard = PlayerDeck[0];
        PlayerDeck.RemoveAt(0);

        CurrentHand.Add(DrawnCard);
        CreateCardWidget(DrawnCard, CurrentHand.Num() - 1);
    }

    OnHandUpdated.Broadcast(CurrentHand, CurrentHand.Num());
}

// ==== DECK MANAGEMENT ====

void AHandManager::SetPlayerDeck(const TArray<int32>& CardIDs)
{
    PlayerDeck.Empty();

    for (int32 CardID : CardIDs)
    {
        FCardData* FoundCard = FindCardByID(CardID);
        if (FoundCard)
        {
            PlayerDeck.Add(*FoundCard);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[KCK HandManager] Card ID %d not found when building deck"), CardID);
        }
    }

    ShuffleDeck();
    UE_LOG(LogTemp, Log, TEXT("[KCK HandManager] Player deck set with %d cards"), PlayerDeck.Num());
}

void AHandManager::AddCardToDeck(int32 CardID)
{
    FCardData* FoundCard = FindCardByID(CardID);
    if (FoundCard)
    {
        PlayerDeck.Add(*FoundCard);
        UE_LOG(LogTemp, Log, TEXT("[KCK HandManager] Added card '%s' to deck"), *FoundCard->Name.ToString());
    }
}

void AHandManager::ShuffleDeck()
{
    // Simple shuffle algorithm
    for (int32 i = PlayerDeck.Num() - 1; i > 0; i--)
    {
        int32 j = FMath::RandRange(0, i);
        PlayerDeck.Swap(i, j);
    }
    UE_LOG(LogTemp, Log, TEXT("[KCK HandManager] Deck shuffled"));
}

// ==== UI MANAGEMENT ====

void AHandManager::SetHandUIPanel(UPanelWidget* Panel)
{
    HandPanel = Panel;
    RefreshHandUI();
}

void AHandManager::RefreshHandUI()
{
    if (!HandPanel || !CardWidgetClass) return;

    // Clear existing widgets
    for (UUserWidget* Widget : HandWidgets)
    {
        if (Widget)
        {
            Widget->RemoveFromParent();
        }
    }
    HandWidgets.Empty();

    // Create new widgets for current hand
    for (int32 i = 0; i < CurrentHand.Num(); i++)
    {
        CreateCardWidget(CurrentHand[i], i);
    }
}

// ==== GAMEPLAY ====

bool AHandManager::PlayCard(int32 HandIndex, AActor* Target)
{
    if (!CurrentHand.IsValidIndex(HandIndex))
    {
        return false;
    }

    FCardData PlayedCard = CurrentHand[HandIndex];

    // Create temporary card actor to execute ability
    if (CardActorClass)
    {
        ACardActor* TempCardActor = GetWorld()->SpawnActor<ACardActor>(CardActorClass);
        if (TempCardActor)
        {
            TempCardActor->InitializeCard(PlayedCard, CardDataTable, AbilityDataTable);
            TempCardActor->ActivateAbility(Target);
            TempCardActor->Destroy();
        }
    }

    // Remove card from hand
    RemoveCardFromHand(HandIndex);

    // Broadcast event
    OnCardPlayed.Broadcast(PlayedCard);

    return true;
}

// ==== UTILITY FUNCTIONS ====

FCardData AHandManager::GetCardInHand(int32 Index) const
{
    if (CurrentHand.IsValidIndex(Index))
    {
        return CurrentHand[Index];
    }
    return FCardData(); // Return empty card data if invalid
}

bool AHandManager::CanPlayCard(int32 HandIndex, int32 CurrentMana) const
{
    if (!CurrentHand.IsValidIndex(HandIndex))
    {
        return false;
    }

    const FCardData& Card = CurrentHand[HandIndex];
    return Card.Cost <= CurrentMana;
}

// ==== PRIVATE HELPER FUNCTIONS ====

FCardData* AHandManager::FindCardByID(int32 CardID)
{
    if (!CardDataTable) return nullptr;

    for (const FName& RowName : CardDataTable->GetRowNames())
    {
        FCardData* FoundCard = CardDataTable->FindRow<FCardData>(RowName, TEXT("FindCardByID"));
        if (FoundCard && FoundCard->ID == CardID)
        {
            return FoundCard;
        }
    }
    return nullptr;
}

void AHandManager::CreateCardWidget(const FCardData& CardData, int32 HandIndex)
{
    if (!HandPanel || !CardWidgetClass) return;

    UUserWidget* CardWidget = CreateWidget<UUserWidget>(GetWorld(), CardWidgetClass);
    if (CardWidget)
    {
        // Set card data on widget (via card interface)
        if (CardWidget->GetClass()->ImplementsInterface(UCardWidget::StaticClass()))
        {
            ICardWidget::Execute_SetCardData(CardWidget, CardData, HandIndex);
        }

        HandPanel->AddChild(CardWidget);

        // Ensure we have enough space in array
        while (HandWidgets.Num() <= HandIndex)
        {
            HandWidgets.Add(nullptr);
        }
        HandWidgets[HandIndex] = CardWidget;
    }
}

void AHandManager::DestroyCardWidget(int32 HandIndex)
{
    if (HandWidgets.IsValidIndex(HandIndex) && HandWidgets[HandIndex])
    {
        HandWidgets[HandIndex]->RemoveFromParent();
        HandWidgets[HandIndex] = nullptr;
    }
}