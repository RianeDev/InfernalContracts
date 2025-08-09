#include "HandManager.h"
#include "CardActor.h"
#include "Engine/World.h"
#include "CombatManager.h"

AHandManager::AHandManager()
{
    PrimaryActorTick.bCanEverTick = false;
    MaxHandSize = 7;
    StartingHandSize = 5;
    
    // Set default CardActorClass
    CardActorClass = ACardActor::StaticClass();
}

void AHandManager::SetCombatManager(ACombatManager* InCombatManager)
{
    CombatManager = InCombatManager;
    UE_LOG(LogTemp, Log, TEXT("[HandManager] CombatManager reference set"));
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
        UE_LOG(LogTemp, Warning, TEXT("[HandManager] Hand is full! Cannot add card ID %d"), CardID);
        return false;
    }

    FCardData* FoundCard = FindCardByID(CardID);
    if (!FoundCard)
    {
        UE_LOG(LogTemp, Warning, TEXT("[HandManager] Card ID %d not found in CardDataTable"), CardID);
        return false;
    }

    // Add to hand
    CurrentHand.Add(*FoundCard);

    // Broadcast individual card added event
    OnCardAddedToHand.Broadcast(*FoundCard);

    // Broadcast overall hand updated event
    OnHandUpdated.Broadcast(CurrentHand, CurrentHand.Num());

    UE_LOG(LogTemp, Log, TEXT("[HandManager] Added card '%s' to hand (Total: %d cards)"),
        *FoundCard->Name.ToString(), CurrentHand.Num());

    return true;
}

bool AHandManager::RemoveCardFromHand(int32 HandIndex)
{
    if (!CurrentHand.IsValidIndex(HandIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("[HandManager] Invalid hand index: %d"), HandIndex);
        return false;
    }

    FCardData RemovedCard = CurrentHand[HandIndex];
    CurrentHand.RemoveAt(HandIndex);

    // Broadcast individual card removed event
    OnCardRemovedFromHand.Broadcast(RemovedCard, HandIndex);

    // Broadcast overall hand updated event
    OnHandUpdated.Broadcast(CurrentHand, CurrentHand.Num());

    UE_LOG(LogTemp, Log, TEXT("[HandManager] Removed card '%s' from hand (Index: %d, Remaining: %d cards)"),
        *RemovedCard.Name.ToString(), HandIndex, CurrentHand.Num());

    return true;
}

void AHandManager::ClearHand()
{
    int32 PreviousSize = CurrentHand.Num();
    CurrentHand.Empty();

    // Broadcast hand updated event
    OnHandUpdated.Broadcast(CurrentHand, 0);

    UE_LOG(LogTemp, Log, TEXT("[HandManager] Hand cleared (%d cards removed)"), PreviousSize);
}

void AHandManager::DrawStartingHand()
{
    ClearHand();
    DrawCards(StartingHandSize);

    UE_LOG(LogTemp, Log, TEXT("[HandManager] Drew starting hand of %d cards"), StartingHandSize);
}

void AHandManager::DrawCards(int32 Count)
{
    int32 CardsDrawn = 0;

    for (int32 i = 0; i < Count; i++)
    {
        if (PlayerDeck.Num() == 0)
        {
            // Try to refill deck from discard pile
            ShuffleDiscardIntoDeck();

            if (PlayerDeck.Num() == 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("[HandManager] Cannot draw card - deck and discard pile empty"));
                break;
            }
        }

        if (CurrentHand.Num() >= MaxHandSize)
        {
            UE_LOG(LogTemp, Warning, TEXT("[HandManager] Cannot draw card - hand is full"));
            break;
        }

        // Draw top card from deck
        FCardData DrawnCard = PlayerDeck[0];
        PlayerDeck.RemoveAt(0);

        CurrentHand.Add(DrawnCard);
        CardsDrawn++;

        // Broadcast individual card added
        OnCardAddedToHand.Broadcast(DrawnCard);
    }

    // Broadcast overall hand update
    OnHandUpdated.Broadcast(CurrentHand, CurrentHand.Num());

    UE_LOG(LogTemp, Log, TEXT("[HandManager] Drew %d cards (Hand: %d, Deck: %d, Discard: %d)"),
        CardsDrawn, CurrentHand.Num(), PlayerDeck.Num(), DiscardPileCardIDs.Num());
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
            UE_LOG(LogTemp, Warning, TEXT("[HandManager] Card ID %d not found when building deck"), CardID);
        }
    }

    ShuffleDeck();
    UE_LOG(LogTemp, Log, TEXT("[HandManager] Player deck set with %d cards"), PlayerDeck.Num());
}

void AHandManager::AddCardToDeck(int32 CardID)
{
    // Get a copy of the card by ID from the card datatable and add a fresh copy to the deck.
    FCardData* FoundCard = FindCardByID(CardID);
    if (FoundCard)
    {
        PlayerDeck.Add(*FoundCard);
        UE_LOG(LogTemp, Log, TEXT("[HandManager] Added card '%s' to deck"), *FoundCard->Name.ToString());
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
    UE_LOG(LogTemp, Log, TEXT("[HandManager] Deck shuffled"));
}

// DISCARD AND BANISH

void AHandManager::AddCardToDiscard(int32 CardID)
{
    DiscardPileCardIDs.Add(CardID);
    // Use the CardID since it will be used later to find from the card datatable to reshuffle into the deck.
    FCardData* FoundCard = FindCardByID(CardID);
    if (FoundCard)
    {
        UE_LOG(LogTemp, Log, TEXT("[HandManager] Card '%s' added to discard pile"), *FoundCard->Name.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[HandManager] Card of CardID %d added to discard pile (no matching data found)"), CardID);
    }
}

void AHandManager::ClearDiscardPile()
{
    DiscardPileCardIDs.Empty();
    UE_LOG(LogTemp, Log, TEXT("[HandManager] Discard pile cleared"));
}

void AHandManager::ShuffleDiscardIntoDeck()
{
    if (DiscardPileCardIDs.Num() == 0)
        return;

    // Get a fresh copy of each discarded card by loading it from the card datatable and adding the copy to the deck
    for (int32 UniqueID : DiscardPileCardIDs)
    {
        FCardData* FoundCard = FindCardByID(UniqueID);
        if (FoundCard)
        {
            PlayerDeck.Add(*FoundCard);
        }
    }

    DiscardPileCardIDs.Empty();
    ShuffleDeck();

    UE_LOG(LogTemp, Log, TEXT("[HandManager] Discard pile shuffled back into deck"));
}

void AHandManager::RemoveCardFromAllPilesByCardID(int32 CardID)
{
    PlayerDeck.RemoveAll([CardID](const FCardData& Card) { return Card.ID == CardID; });
    CurrentHand.RemoveAll([CardID](const FCardData& Card) { return Card.ID == CardID; });
    DiscardPileCardIDs.RemoveAll([CardID](int32 ID) { return ID == CardID; });

    UE_LOG(LogTemp, Log, TEXT("[HandManager] Card ID %d removed from all piles"), CardID);
}

void AHandManager::BanishCardByID(int32 CardID)
{
    if (!BanishedCardIDs.Contains(CardID))
    {
        BanishedCardIDs.Add(CardID);
        UE_LOG(LogTemp, Log, TEXT("[HandManager] Card ID %d added to banished pile"), CardID);
    }

    RemoveCardFromAllPilesByCardID(CardID);
}


// ==== GAMEPLAY ====

bool AHandManager::PlayCard(int32 HandIndex, AActor* Target)
{
    if (!CurrentHand.IsValidIndex(HandIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("[HandManager] PlayCard: Invalid hand index %d"), HandIndex);
        return false;
    }

    FCardData PlayedCard = CurrentHand[HandIndex];

    UE_LOG(LogTemp, Log, TEXT("[HandManager] Playing card: %s at index %d"),
        *PlayedCard.Name.ToString(), HandIndex);

    // Step 1: Deal damage if card has Attack > 0
    if (PlayedCard.Attack > 0)
    {
        if (CombatManager)
        {
            UE_LOG(LogTemp, Log, TEXT("[HandManager] Card has %d Attack - dealing damage to enemy"), PlayedCard.Attack);
            CombatManager->DamageEnemy(PlayedCard.Attack);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[HandManager] Cannot deal damage - CombatManager is null"));
        }
    }

    // Step 2: Execute ability if card has one
    if (CardActorClass)
    {
        UE_LOG(LogTemp, Log, TEXT("[HandManager] Creating CardActor for ability execution"));
        ACardActor* TempCardActor = GetWorld()->SpawnActor<ACardActor>(CardActorClass);
        if (TempCardActor)
        {
            UE_LOG(LogTemp, Log, TEXT("[HandManager] CardActor created successfully"));
            TempCardActor->InitializeCard(PlayedCard, CardDataTable, AbilityDataTable);
            TempCardActor->ActivateAbility(Target);
            TempCardActor->Destroy();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[HandManager] Failed to create CardActor"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[HandManager] No CardActorClass set - ability will not execute"));
    }

    // Remove card from hand (this will broadcast the removal automatically)
    RemoveCardFromHand(HandIndex);

    // Broadcast card played event
    OnCardPlayed.Broadcast(PlayedCard);

    UE_LOG(LogTemp, Log, TEXT("[HandManager] Successfully played card: %s"), *PlayedCard.Name.ToString());

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

bool AHandManager::CanPlayCard(int32 HandIndex, int32 CurrentEnergy) const
{
    if (!CurrentHand.IsValidIndex(HandIndex))
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("[HandManager] CanPlayCard: Invalid hand index %d"), HandIndex);
        return false;
    }

    const FCardData& Card = CurrentHand[HandIndex];

    // Champion cards (cost 0) should always be playable when combat allows
    if (Card.CardType == ECardType::Champion && Card.Cost == 0)
    {
        return true;
    }

    bool bCanAfford = Card.Cost <= CurrentEnergy;

    UE_LOG(LogTemp, VeryVerbose, TEXT("[HandManager] CanPlayCard: %s (Cost: %d, Available: %d) = %s"),
        *Card.Name.ToString(), Card.Cost, CurrentEnergy, bCanAfford ? TEXT("Yes") : TEXT("No"));

    return bCanAfford;
}

// ==== PRIVATE HELPER FUNCTIONS ====

FCardData* AHandManager::FindCardByID(int32 CardID)
{
    // This is only for finding BASE data from the card datatable, NOT for tracking realtime cards!
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