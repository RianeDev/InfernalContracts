#include "CombatManager.h"
#include "HandManager.h"
#include "Blueprint/UserWidget.h"
#include "CombatUIWidget.h"
#include "PaperCharacter.h"
#include "Engine/World.h"
#include "TimerManager.h"

// Initialize static variable
int32 ACombatManager::NextUniqueCardID = 0;

ACombatManager::ACombatManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Default Player Health stats (persistent across combats)
    PlayerHealth = 20;
    PlayerMaxHealth = 20;

    // Default energy per turn
    CurrentEnergy = 3;
    MaxEnergyPerTurn = 3;
    PlayerFaction = ECardFaction::Demon;
}

void ACombatManager::BeginPlay()
{
    Super::BeginPlay();
}

void ACombatManager::SetCombatUI(UUserWidget* InCombatUI)
{
    CombatUI = InCombatUI;

    if (CombatUI && HandManager)
    {
        if (UCombatUIWidget* TypedUI = Cast<UCombatUIWidget>(CombatUI))
        {
            TypedUI->InitializeUI(this, HandManager);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatManager] CombatUI set but HandManager is missing, will bind later"));
    }
}

void ACombatManager::StartCombat(const FEnemyData& Enemy, const TArray<int32>& PlayerDeckIDs)
{
    if (!HandManager)
    {
        UE_LOG(LogTemp, Error, TEXT("[CombatManager] No HandManager assigned!"));
        return;
    }

    CurrentEnemy = Enemy;

    // Store a reference to enemy actor
    // TODO: Use actual custom class for enemy
    EnemyActor = Cast<APaperCharacter>(Enemy.ActorReference); // or pass this as part of Enemy struct

    if (EnemyActor)
    {
        UEnemyAIComponent* FoundEnemyAIComp = EnemyActor->FindComponentByClass<UEnemyAIComponent>();
        if (FoundEnemyAIComp)
        {
            EnemyAIComponent = FoundEnemyAIComp;
            EnemyAIComponent->SetCombatManager(this);
            EnemyAIComponent->InitializeEnemyAI(Enemy.EnemyDeckCardIDs);
            EnemyAIComponent->ResetEnergy();
            EnemyAIComponent->OnEnemyHealthChanged.AddDynamic(this, &ACombatManager::HandleEnemyHealthChanged);

        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[CombatManager] EnemyAIComponent not found on enemy actor %s"), *EnemyActor->GetName());
            return;
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatManager] EnemyActor reference is null."));
        return;
    }

    // Clear battlefields
    PlayerBattlefield.Empty();
    EnemyBattlefield.Empty();

    // Ensure we have an existing CombatUI
    if (!CombatUI)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatManager] No existing CombatUI widget found! UI will not update."));
    }
    else
    {
        // Bind UI to managers BEFORE drawing cards so playability is calculated correctly
        if (UCombatUIWidget* TypedCombatUI = Cast<UCombatUIWidget>(CombatUI))
        {
            TypedCombatUI->InitializeUI(this, HandManager);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[CombatManager] CombatUI is not of type UCombatUIWidget."));
        }
    }

    // Set up player deck and draw starting hand (now UI is already listening if it exists)
    HandManager->SetPlayerDeck(PlayerDeckIDs);
    HandManager->DrawStartingHand();

    // Set up cross-references between managers
    HandManager->SetCombatManager(this);

    // Bind to hand manager events
    if (!HandManager->OnCardPlayed.IsBound())
    {
        HandManager->OnCardPlayed.AddDynamic(this, &ACombatManager::OnCardPlayed);
    }

    // Set starting energy based on faction
    CurrentEnergy = MaxEnergyPerTurn;

    // Start combat in "Starting" phase
    SetCombatState(ECombatState::Starting);

    // Brief delay then start player turn
    FTimerHandle DelayTimer;
    GetWorld()->GetTimerManager().SetTimer(DelayTimer, [this]()
        {
            SetCombatState(ECombatState::PlayerTurn);
        }, 1.0f, false);

    UE_LOG(LogTemp, Log, TEXT("[CombatManager] Combat started against %s"), *CurrentEnemy.Name.ToString());
}

void ACombatManager::EndCombat(bool bPlayerWon)
{
    SetCombatState(bPlayerWon ? ECombatState::Victory : ECombatState::Defeat);

    PlayerBattlefield.Empty();
    EnemyBattlefield.Empty();

    if (CombatUI)
    {
        CombatUI->RemoveFromParent();
        CombatUI = nullptr;
    }

    if (HandManager)
    {
        HandManager->ClearHand();
        HandManager->ClearDiscardPile();
    }

    // Return banished cards to player collection/deck here
    HandManager->BanishedCardIDs.Empty();

    UE_LOG(LogTemp, Log, TEXT("[CombatManager] Combat ended - Player %s"), bPlayerWon ? TEXT("Won") : TEXT("Lost"));
}


void ACombatManager::EndPlayerTurn()
{
    if (CurrentState != ECombatState::PlayerTurn)
    {
        return;
    }

    // According to your rules: "When you end your turn, you discard all cards in your hand"
    if (HandManager)
    {
        HandManager->ClearHand();

        // Check if deck has cards before shuffling and drawing
        if (HandManager->GetDeckSize() > 0)
        {
            // Shuffle deck and draw new hand for next turn
            HandManager->ShuffleDeck();
            HandManager->DrawCards(HandManager->StartingHandSize);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[CombatManager] Cannot draw new hand - deck is empty"));
        }
    }

    // Reset energy for next turn  
    CurrentEnergy = MaxEnergyPerTurn;

    SetCombatState(ECombatState::EnemyTurn);

    // Process enemy turn after a brief delay
    FTimerHandle EnemyTurnTimer;
    GetWorld()->GetTimerManager().SetTimer(EnemyTurnTimer, this, &ACombatManager::ProcessEnemyTurn, 1.5f, false);
}

// Enhanced damage function - targets battlefield cards first
void ACombatManager::ModifyPlayerHealth(int32 HealthDelta)
{
    if (HealthDelta == 0)
        return;

    if (HealthDelta < 0)
    {
        int32 Damage = -HealthDelta;

        UE_LOG(LogTemp, Log, TEXT("[CombatManager] Attempting to damage player for %d"), Damage);

        // First, try to damage a player's battlefield card with health
        if (DamageFirstAvailablePlayerCard(Damage))
        {
            UE_LOG(LogTemp, Log, TEXT("[CombatManager] Damage applied to player's battlefield card"));
            return;
        }

        // If no cards available, damage player directly
        PlayerHealth = FMath::Max(0, PlayerHealth - Damage);
        OnHealthChanged.Broadcast(true, PlayerHealth);

        UE_LOG(LogTemp, Log, TEXT("[CombatManager] No player cards available - damage applied to player health, health now %d"), PlayerHealth);
    }
    else
    {
        // Healing
        PlayerHealth = FMath::Min(PlayerMaxHealth, PlayerHealth + HealthDelta);
        OnHealthChanged.Broadcast(true, PlayerHealth);

        UE_LOG(LogTemp, Log, TEXT("[CombatManager] Player Health heals %d, health now %d"), HealthDelta, PlayerHealth);
    }

    CheckWinConditions();
}


void ACombatManager::DamageEnemy(int32 Damage)
{
    if (Damage <= 0) return;

    if (EnemyAIComponent)
    {
        EnemyAIComponent->ModifyEnemyHealth(-Damage);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatManager] No EnemyAIComponent - apply damage directly"));

        CurrentEnemy.Health = FMath::Max(0, CurrentEnemy.Health - Damage);
        OnHealthChanged.Broadcast(false, CurrentEnemy.Health);
        CheckWinConditions();
    }
}


bool ACombatManager::DamageFirstAvailableEnemyCard(int32 Damage)
{
    for (int32 i = 0; i < EnemyBattlefield.Num(); i++)
    {
        if (EnemyBattlefield[i].CurrentHealth > 0)
        {
            return DamageSpecificBattlefieldCard(i, false, Damage);
        }
    }
    return false;
}

void ACombatManager::SetPlayerEnergy(int32 NewEnergy)
{
    CurrentEnergy = FMath::Clamp(NewEnergy, 0, 99); // No hard cap mentioned in rules
    UE_LOG(LogTemp, Log, TEXT("[CombatManager] Player energy set to %d"), CurrentEnergy);
}

void ACombatManager::SpendEnergy(int32 Cost)
{
    CurrentEnergy = FMath::Max(0, CurrentEnergy - Cost);
    UE_LOG(LogTemp, Log, TEXT("[CombatManager] Player spends %d energy, %d remaining"), Cost, CurrentEnergy);
}

// Battlefield management functions
void ACombatManager::SummonCreature(const FCardData& CreatureCard, bool bIsPlayerOwned)
{
    if (CreatureCard.CardType != ECardType::Creature && CreatureCard.CardType != ECardType::Champion)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatManager] Cannot summon non-creature card: %s"), *CreatureCard.Name.ToString());
        return;
    }

    FBattlefieldCard BattlefieldCard(CreatureCard, bIsPlayerOwned);
    BattlefieldCard.UniqueID = ++NextUniqueCardID; // Assign unique ID

    int32 NewIndex = INDEX_NONE;
    if (bIsPlayerOwned)
    {
        PlayerBattlefield.Add(BattlefieldCard);
        NewIndex = PlayerBattlefield.Num() - 1;
        PlayerBattlefield[NewIndex].BattlefieldIndex = NewIndex; // Set array index

        UE_LOG(LogTemp, Log, TEXT("[CombatManager] Player summoned %s (ID:%d, Index:%d, %d ATK/%d HP)"),
            *CreatureCard.Name.ToString(), BattlefieldCard.UniqueID, NewIndex, CreatureCard.Attack, CreatureCard.Health);
    }
    else
    {
        EnemyBattlefield.Add(BattlefieldCard);
        NewIndex = EnemyBattlefield.Num() - 1;
        EnemyBattlefield[NewIndex].BattlefieldIndex = NewIndex; // Set array index

        UE_LOG(LogTemp, Log, TEXT("[CombatManager] Enemy summoned %s (ID:%d, Index:%d, %d ATK/%d HP)"),
            *CreatureCard.Name.ToString(), BattlefieldCard.UniqueID, NewIndex, CreatureCard.Attack, CreatureCard.Health);
    }

    // Fire summon event for Blueprints
    OnCreatureSummoned.Broadcast(BattlefieldCard, NewIndex, bIsPlayerOwned);
}

void ACombatManager::RemoveCardFromBattlefield(int32 BattlefieldIndex, bool bIsPlayerSide)
{
    TArray<FBattlefieldCard>& Battlefield = bIsPlayerSide ? PlayerBattlefield : EnemyBattlefield;

    if (!Battlefield.IsValidIndex(BattlefieldIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatManager] Invalid battlefield index: %d"), BattlefieldIndex);
        return;
    }

    // Store card info before removal
    FBattlefieldCard CardToRemove = Battlefield[BattlefieldIndex];
    FString CardName = CardToRemove.CardData.Name.ToString();
    int32 UniqueID = CardToRemove.UniqueID;
    int32 CardDefID = CardToRemove.CardData.ID;

    // Remove from array
    Battlefield.RemoveAt(BattlefieldIndex);

    // Update indices for remaining cards
    UpdateBattlefieldIndices();

    UE_LOG(LogTemp, Log, TEXT("[CombatManager] Removed card '%s' (ID:%d, DefID:%d) at index %d from %s battlefield"),
        *CardName, UniqueID, CardDefID, BattlefieldIndex, bIsPlayerSide ? TEXT("player") : TEXT("enemy"));

    // If creature died (health <= 0), send its card definition to the discard pile
    if (CardToRemove.CurrentHealth <= 0)
    {
        if (HandManager)
        {
            HandManager->AddCardToDiscard(CardToRemove.CardData.ID);
            UE_LOG(LogTemp, Log, TEXT("[CombatManager] Creature '%s' (DefID:%d) discarded after death"), *CardName, CardDefID);
        }
    }

    // Fire remove event for Blueprints
    OnCreatureRemoved.Broadcast(BattlefieldIndex, bIsPlayerSide);
}


void ACombatManager::BanishCard(const FCardData& CardToBanish)
{
    if (!HandManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatManager] Cannot banish card '%s' because HandManager is null"), *CardToBanish.Name.ToString());
        return;
    }

    // Banished in HandManager (deck/hand/discard)
    HandManager->BanishCardByID(CardToBanish.ID);

    UE_LOG(LogTemp, Log, TEXT("[CombatManager] Card '%s' (ID: %d) banished for this combat"), *CardToBanish.Name.ToString(), CardToBanish.ID);

    // Check if card is summoned on battlefield (player or enemy)
    int32 UniqueID = FindUniqueIDByCardID(CardToBanish.ID, true);
    bool bWasSummoned = false;

    if (UniqueID != INDEX_NONE)
    {
        RemoveBattlefieldCardByUniqueID(UniqueID);
        bWasSummoned = true;
    }
    else
    {
        UniqueID = FindUniqueIDByCardID(CardToBanish.ID, false);
        if (UniqueID != INDEX_NONE)
        {
            RemoveBattlefieldCardByUniqueID(UniqueID);
            bWasSummoned = true;
        }
    }

    // Broadcast event with summoned info and UniqueID or -1
    OnCardBanished.Broadcast(CardToBanish, bWasSummoned, bWasSummoned ? UniqueID : -1);
}


int32 ACombatManager::FindUniqueIDByCardID(int32 CardID, bool bIsPlayerSide) const
{
    const TArray<FBattlefieldCard>& Battlefield = bIsPlayerSide ? PlayerBattlefield : EnemyBattlefield;

    for (const FBattlefieldCard& Card : Battlefield)
    {
        if (Card.CardData.ID == CardID)
        {
            return Card.UniqueID;
        }
    }
    return INDEX_NONE;
}

bool ACombatManager::DamageSpecificBattlefieldCard(int32 BattlefieldIndex, bool bIsPlayerSide, int32 Damage)
{
    TArray<FBattlefieldCard>& Battlefield = bIsPlayerSide ? PlayerBattlefield : EnemyBattlefield;

    if (!Battlefield.IsValidIndex(BattlefieldIndex) || Damage <= 0)
    {
        return false;
    }

    FBattlefieldCard& Card = Battlefield[BattlefieldIndex];
    int32 UniqueID = Card.UniqueID; // Store the unique ID
    Card.CurrentHealth = FMath::Max(0, Card.CurrentHealth - Damage);

    UE_LOG(LogTemp, Log, TEXT("[CombatManager] %s (ID:%d, Index:%d) takes %d damage, health now %d"),
        *Card.CardData.Name.ToString(), UniqueID, BattlefieldIndex, Damage, Card.CurrentHealth);

    // Broadcast events
    OnCardDamaged.Broadcast(UniqueID, Damage, bIsPlayerSide);

    // Remove card if health reaches 0
    if (Card.CurrentHealth <= 0)
    {
        UE_LOG(LogTemp, Log, TEXT("[CombatManager] %s (ID:%d) destroyed"),
            *Card.CardData.Name.ToString(), UniqueID);
        RemoveCardFromBattlefield(BattlefieldIndex, bIsPlayerSide);
    }

    return true;
}

// NEW: Damage by Unique ID
bool ACombatManager::DamageBattlefieldCardByUniqueID(int32 UniqueID, int32 Damage)
{
    if (Damage <= 0 || UniqueID <= 0) return false;

    // Check player battlefield first
    int32 PlayerIndex = FindBattlefieldIndexByUniqueID(UniqueID, true);
    if (PlayerIndex != INDEX_NONE)
    {
        return DamageSpecificBattlefieldCard(PlayerIndex, true, Damage);
    }

    // Check enemy battlefield
    int32 EnemyIndex = FindBattlefieldIndexByUniqueID(UniqueID, false);
    if (EnemyIndex != INDEX_NONE)
    {
        return DamageSpecificBattlefieldCard(EnemyIndex, false, Damage);
    }

    UE_LOG(LogTemp, Warning, TEXT("[CombatManager] No card found with Unique ID: %d"), UniqueID);
    return false;
}

// NEW: Get card by Unique ID
FBattlefieldCard ACombatManager::GetBattlefieldCardByUniqueID(int32 UniqueID, bool& bFound) const
{
    bFound = false;

    // Check player battlefield
    for (const FBattlefieldCard& Card : PlayerBattlefield)
    {
        if (Card.UniqueID == UniqueID)
        {
            bFound = true;
            return Card;
        }
    }

    // Check enemy battlefield
    for (const FBattlefieldCard& Card : EnemyBattlefield)
    {
        if (Card.UniqueID == UniqueID)
        {
            bFound = true;
            return Card;
        }
    }

    return FBattlefieldCard(); // Return empty card if not found
}

// NEW: Remove by Unique ID
bool ACombatManager::RemoveBattlefieldCardByUniqueID(int32 UniqueID)
{
    if (UniqueID <= 0) return false;

    // Check player battlefield
    int32 PlayerIndex = FindBattlefieldIndexByUniqueID(UniqueID, true);
    if (PlayerIndex != INDEX_NONE)
    {
        RemoveCardFromBattlefield(PlayerIndex, true);
        return true;
    }

    // Check enemy battlefield
    int32 EnemyIndex = FindBattlefieldIndexByUniqueID(UniqueID, false);
    if (EnemyIndex != INDEX_NONE)
    {
        RemoveCardFromBattlefield(EnemyIndex, false);
        return true;
    }

    return false;
}

bool ACombatManager::HasPlayerCardsWithHealth() const
{
    for (const FBattlefieldCard& Card : PlayerBattlefield)
    {
        if (Card.CurrentHealth > 0)
        {
            return true;
        }
    }
    return false;
}

bool ACombatManager::HasEnemyCardsWithHealth() const
{
    for (const FBattlefieldCard& Card : EnemyBattlefield)
    {
        if (Card.CurrentHealth > 0)
        {
            return true;
        }
    }
    return false;
}

// Private helper functions for damage targeting
bool ACombatManager::DamageFirstAvailablePlayerCard(int32 Damage)
{
    for (int32 i = 0; i < PlayerBattlefield.Num(); i++)
    {
        if (PlayerBattlefield[i].CurrentHealth > 0)
        {
            return DamageSpecificBattlefieldCard(i, true, Damage);
        }
    }
    return false; // No cards with health found
}

void ACombatManager::SetCombatState(ECombatState NewState)
{
    if (CurrentState == NewState) return;

    ECombatState OldState = CurrentState;
    CurrentState = NewState;

    OnCombatStateChanged.Broadcast(CurrentState);

    UE_LOG(LogTemp, Log, TEXT("[CombatManager] Combat state changed from %d to %d"),
        (int32)OldState, (int32)CurrentState);
}

void ACombatManager::ProcessEnemyTurn()
{
    if (CurrentState != ECombatState::EnemyTurn)
    {
        return;
    }

    if (EnemyAIComponent)
    {
        EnemyAIComponent->OnEnemyAITurnEnded.RemoveDynamic(this, &ACombatManager::OnEnemyTurnComplete);
        EnemyAIComponent->OnEnemyAITurnEnded.AddDynamic(this, &ACombatManager::OnEnemyTurnComplete);


        EnemyAIComponent->StartEnemyTurn();
    }
    else
    {
        // Fallback or simple AI damage logic
        ModifyPlayerHealth(-FMath::RandRange(10, 20));
        CheckWinConditions();
        SetCombatState(ECombatState::PlayerTurn);
    }
}

void ACombatManager::OnEnemyTurnComplete()
{
    EnemyAIComponent->OnEnemyAITurnEnded.RemoveDynamic(this, &ACombatManager::OnEnemyTurnComplete);
    EnemyAIComponent->OnEnemyAITurnEnded.AddDynamic(this, &ACombatManager::OnEnemyTurnComplete);


    if (IsCombatActive())
    {
        SetCombatState(ECombatState::PlayerTurn);
    }
}

void ACombatManager::CheckWinConditions()
{
    if (PlayerHealth <= 0)
    {
        EndCombat(false); // Player lost - Player Health destroyed
    }
    else if (CurrentEnemy.Health <= 0)
    {
        EndCombat(true); // Player won - Enemy defeated
    }
}

void ACombatManager::OnCardPlayed(const FCardData& PlayedCard)
{
    // Spend energy for the card
    SpendEnergy(PlayedCard.Cost);

    // Apply card effects based on type
    if (PlayedCard.CardType == ECardType::Creature || PlayedCard.CardType == ECardType::Champion)
    {
        // Creatures and Champions go to the battlefield
        UE_LOG(LogTemp, Log, TEXT("[CombatManager] Summoning creature '%s'"), *PlayedCard.Name.ToString());
        SummonCreature(PlayedCard, true);
    }
    else if (PlayedCard.CardType == ECardType::Spell)
    {
        // Spells have immediate effects then are discarded
        // You'll implement specific spell effects based on AbilityID
        UE_LOG(LogTemp, Log, TEXT("[CombatManager] Spell '%s' cast"), *PlayedCard.Name.ToString());
    }
    else if (PlayedCard.CardType == ECardType::Power)
    {
        // Powers are permanent for this combat
        UE_LOG(LogTemp, Log, TEXT("[CombatManager] Power '%s' activated (permanent this combat)"), *PlayedCard.Name.ToString());
    }
    else if (PlayedCard.CardType == ECardType::Skill)
    {
        // Skills have immediate effects then are discarded
        UE_LOG(LogTemp, Log, TEXT("[CombatManager] Skill '%s' used"), *PlayedCard.Name.ToString());
    }

    UE_LOG(LogTemp, Log, TEXT("[CombatManager] Card played: %s (Cost: %d Energy)"),
        *PlayedCard.Name.ToString(), PlayedCard.Cost);
}

// Helper function to update battlefield indices after removal
void ACombatManager::UpdateBattlefieldIndices()
{
    // Update player battlefield indices
    for (int32 i = 0; i < PlayerBattlefield.Num(); i++)
    {
        PlayerBattlefield[i].BattlefieldIndex = i;
    }

    // Update enemy battlefield indices
    for (int32 i = 0; i < EnemyBattlefield.Num(); i++)
    {
        EnemyBattlefield[i].BattlefieldIndex = i;
    }

    UE_LOG(LogTemp, VeryVerbose, TEXT("[CombatManager] Updated battlefield indices - Player: %d, Enemy: %d"),
        PlayerBattlefield.Num(), EnemyBattlefield.Num());
}

// Helper function to find battlefield index by unique ID
int32 ACombatManager::FindBattlefieldIndexByUniqueID(int32 UniqueID, bool bIsPlayerSide) const
{
    const TArray<FBattlefieldCard>& Battlefield = bIsPlayerSide ? PlayerBattlefield : EnemyBattlefield;

    for (int32 i = 0; i < Battlefield.Num(); i++)
    {
        if (Battlefield[i].UniqueID == UniqueID)
        {
            return i;
        }
    }

    return INDEX_NONE; // Not found
}

// Play enemy card (called from EnemyAIComponent)
bool ACombatManager::PlayEnemyCard(const FCardData& CardToPlay)
{
    UE_LOG(LogTemp, Log, TEXT("[CombatManager] Enemy plays card %s"), *CardToPlay.Name.ToString());

    // Apply card effects similar to player playing cards
    if (CardToPlay.CardType == ECardType::Creature || CardToPlay.CardType == ECardType::Champion)
    {
        SummonCreature(CardToPlay, false);
    }
    else if (CardToPlay.CardType == ECardType::Spell)
    {
        // TODO: Spell effect logic for enemy
    }
    else if (CardToPlay.CardType == ECardType::Power)
    {
        // TODO: Power effect for enemy
    }
    else if (CardToPlay.CardType == ECardType::Skill)
    {
        // TODO: Skill effect for enemy
    }

    return true;
}



void ACombatManager::HandleEnemyHealthChanged(int32 NewHealth)
{
    CurrentEnemy.Health = NewHealth;
    OnHealthChanged.Broadcast(false, NewHealth);  // false = enemy

    CheckWinConditions();
}
