#include "EnemyAIComponent.h"
#include "CombatManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

UEnemyAIComponent::UEnemyAIComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    CurrentEnergy = 0;
    Health = MaxHealth = 100; // Set reasonable default or initialize later
    NextCardToPlayIndex = 0;
    bIsEnemyTurnActive = false;
}

void UEnemyAIComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UEnemyAIComponent::InitializeEnemyAI(const TArray<int32>& DeckCardIDs)
{
    EnemyDeck.Empty();
    EnemyHand.Empty();
    DiscardPileCardIDs.Empty();

    for (int32 CardID : DeckCardIDs)
    {
        if (FCardData* FoundCard = FindCardByID(CardID))
        {
            EnemyDeck.Add(*FoundCard);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[EnemyAI] CardID %d not found in CardDataTable"), CardID);
        }
    }

    ShuffleDeck();
    DrawCards(5); // Starting hand size

    CurrentEnergy = MaxEnergyPerTurn;
}

void UEnemyAIComponent::SetCombatManager(ACombatManager* InCombatManager)
{
    CombatManager = InCombatManager;
}

FCardData* UEnemyAIComponent::FindCardByID(int32 CardID) const
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

void UEnemyAIComponent::ShuffleDeck()
{
    for (int32 i = EnemyDeck.Num() - 1; i > 0; i--)
    {
        int32 j = FMath::RandRange(0, i);
        EnemyDeck.Swap(i, j);
    }
}

void UEnemyAIComponent::DrawCards(int32 Count)
{
    for (int32 i = 0; i < Count; i++)
    {
        if (EnemyDeck.Num() == 0)
        {
            ShuffleDiscardIntoDeck();
            if (EnemyDeck.Num() == 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("[EnemyAI] Deck and discard empty, cannot draw more cards"));
                break;
            }
        }

        if (EnemyHand.Num() >= 7) // Max hand size
        {
            UE_LOG(LogTemp, Warning, TEXT("[EnemyAI] Hand is full, cannot draw more cards"));
            break;
        }

        EnemyHand.Add(EnemyDeck[0]);
        EnemyDeck.RemoveAt(0);
    }
}

void UEnemyAIComponent::ShuffleDiscardIntoDeck()
{
    if (DiscardPileCardIDs.Num() == 0) return;

    for (int32 CardID : DiscardPileCardIDs)
    {
        if (FCardData* FoundCard = FindCardByID(CardID))
        {
            EnemyDeck.Add(*FoundCard);
        }
    }
    DiscardPileCardIDs.Empty();
    ShuffleDeck();
}

void UEnemyAIComponent::AddCardToDiscard(int32 CardID)
{
    DiscardPileCardIDs.Add(CardID);
}

void UEnemyAIComponent::ClearHand()
{
    EnemyHand.Empty();
}

void UEnemyAIComponent::SetCurrentEnergy(int32 NewEnergy)
{
    CurrentEnergy = FMath::Clamp(NewEnergy, 0, MaxEnergyPerTurn);
}

bool UEnemyAIComponent::TryPlayCard(int32 HandIndex)
{
    if (!EnemyHand.IsValidIndex(HandIndex) || !CombatManager)
    {
        return false;
    }

    const FCardData& CardToPlay = EnemyHand[HandIndex];

    if (CardToPlay.Cost > CurrentEnergy)
    {
        return false; // Can't afford
    }

    bool bSuccess = CombatManager->PlayEnemyCard(CardToPlay);

    if (bSuccess)
    {
        CurrentEnergy -= CardToPlay.Cost;
        OnEnemyAIAttemptedPlay.Broadcast(CardToPlay);
        EnemyHand.RemoveAt(HandIndex);
    }

    return bSuccess;
}

void UEnemyAIComponent::StartEnemyTurn()
{
    CurrentEnergy = MaxEnergyPerTurn;
    NextCardToPlayIndex = 0;
    bIsEnemyTurnActive = true;

    ClearHand();
    DrawCards(5);

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(EnemyTurnStepTimerHandle, this, &UEnemyAIComponent::ProcessEnemyTurnStep, 2.5f, true);
    }
}

void UEnemyAIComponent::ProcessEnemyTurnStep()
{
    if (!bIsEnemyTurnActive)
        return;

    if (EnemyHand.Num() == 0)
    {
        EndTurn();
        return;
    }

    int32 CardIndex = SelectCardToPlay();

    if (CardIndex == -1)
    {
        EndTurn();
        return;
    }

    bool bPlayed = TryPlayCard(CardIndex);

    if (!bPlayed)
    {
        // Failed to play card, try next card next tick
        NextCardToPlayIndex = (NextCardToPlayIndex + 1) % EnemyHand.Num();
    }
    else
    {
        // Played card, update NextCardToPlayIndex safely
        if (EnemyHand.Num() > 0)
        {
            NextCardToPlayIndex = NextCardToPlayIndex % EnemyHand.Num();
        }
        else
        {
            EndTurn();
        }
    }
}

int32 UEnemyAIComponent::SelectCardToPlay_Implementation()
{
    if (EnemyHand.Num() == 0)
    {
        return -1;
    }

    for (int32 i = 0; i < EnemyHand.Num(); i++)
    {
        int32 IndexToTry = (NextCardToPlayIndex + i) % EnemyHand.Num();
        if (EnemyHand[IndexToTry].Cost <= CurrentEnergy)
        {
            return IndexToTry;
        }
    }

    return -1;
}

void UEnemyAIComponent::EndTurn()
{
    bIsEnemyTurnActive = false;

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(EnemyTurnStepTimerHandle);
    }

    ClearHand();

    OnEnemyAITurnEnded.Broadcast();
}

void UEnemyAIComponent::ModifyEnemyHealth(int32 HealthDelta)
{
    if (HealthDelta == 0)
        return;

    if (HealthDelta < 0)
    {
        int32 Damage = -HealthDelta;

        UE_LOG(LogTemp, Log, TEXT("[EnemyAI] Attempting to damage enemy for %d"), Damage);

        if (DamageFirstAvailableEnemyCard(Damage))
        {
            UE_LOG(LogTemp, Log, TEXT("[EnemyAI] Damage applied to enemy battlefield card"));
            return;
        }

        Health = FMath::Max(0, Health - Damage);
    }
    else
    {
        Health = FMath::Min(MaxHealth, Health + HealthDelta);
    }

    OnEnemyHealthChanged.Broadcast(Health);
}

bool UEnemyAIComponent::DamageFirstAvailableEnemyCard(int32 Damage)
{
    if (CombatManager)
    {
        return CombatManager->DamageFirstAvailableEnemyCard(Damage);
    }
    return false;
}

FCardData UEnemyAIComponent::GetCardInHand(int32 Index) const
{
    if (EnemyHand.IsValidIndex(Index))
    {
        return EnemyHand[Index];
    }
    return FCardData();
}
