#include "CombatManager.h"
#include "HandManager.h"
#include "Blueprint/UserWidget.h"
#include "CombatUIWidget.h"
#include "Engine/World.h"
#include "TimerManager.h"

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

    // Set up enemy
    CurrentEnemy = Enemy;

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

    // Clean up battlefields
    PlayerBattlefield.Empty();
    EnemyBattlefield.Empty();

    // Clean up UI
    if (CombatUI)
    {
        CombatUI->RemoveFromParent();
        CombatUI = nullptr;
    }

    // Clear hand
    if (HandManager)
    {
        HandManager->ClearHand();
    }

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
void ACombatManager::DamagePlayer(int32 Damage)
{
    if (Damage <= 0) return;

    UE_LOG(LogTemp, Log, TEXT("[CombatManager] Attempting to damage player for %d"), Damage);

    // First, try to damage a player's card with health
    if (DamageFirstAvailablePlayerCard(Damage))
    {
        UE_LOG(LogTemp, Log, TEXT("[CombatManager] Damage applied to player's battlefield card"));
        return;
    }

    // If no cards available, damage player directly
    DamagePlayerHealth(Damage);
    UE_LOG(LogTemp, Log, TEXT("[CombatManager] No player cards available - damage applied to player health"));
}

void ACombatManager::DamageEnemy(int32 Damage)
{
    if (Damage <= 0) return;

    UE_LOG(LogTemp, Log, TEXT("[CombatManager] Attempting to damage enemy for %d"), Damage);

    // First, try to damage an enemy's card with health
    if (DamageFirstAvailableEnemyCard(Damage))
    {
        UE_LOG(LogTemp, Log, TEXT("[CombatManager] Damage applied to enemy's battlefield card"));
        return;
    }

    // If no cards available, damage enemy directly
    CurrentEnemy.Health = FMath::Max(0, CurrentEnemy.Health - Damage);
    OnHealthChanged.Broadcast(false, CurrentEnemy.Health); // false = is enemy

    UE_LOG(LogTemp, Log, TEXT("[CombatManager] No enemy cards available - %s takes %d damage, health now %d"),
        *CurrentEnemy.Name.ToString(), Damage, CurrentEnemy.Health);

    CheckWinConditions();
}

void ACombatManager::HealPlayer(int32 Amount)
{
    // Healing affects the Player Health
    HealPlayerHealth(Amount);
}

void ACombatManager::DamagePlayerHealth(int32 Damage)
{
    if (Damage <= 0) return;

    PlayerHealth = FMath::Max(0, PlayerHealth - Damage);
    OnHealthChanged.Broadcast(true, PlayerHealth); // true = is player's Player Health

    UE_LOG(LogTemp, Log, TEXT("[CombatManager] Player Health takes %d damage, health now %d"), Damage, PlayerHealth);

    CheckWinConditions();
}

void ACombatManager::HealPlayerHealth(int32 Amount)
{
    if (Amount <= 0) return;

    PlayerHealth = FMath::Min(PlayerMaxHealth, PlayerHealth + Amount);
    OnHealthChanged.Broadcast(true, PlayerHealth);

    UE_LOG(LogTemp, Log, TEXT("[CombatManager] Player Health heals %d, health now %d"), Amount, PlayerHealth);
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

    int32 NewIndex = INDEX_NONE;
    if (bIsPlayerOwned)
    {
        PlayerBattlefield.Add(BattlefieldCard);
        NewIndex = PlayerBattlefield.Num() - 1;

        UE_LOG(LogTemp, Log, TEXT("[CombatManager] Player summoned %s (%d ATK/%d HP) at index %d"),
            *CreatureCard.Name.ToString(), CreatureCard.Attack, CreatureCard.Health, NewIndex);
    }
    else
    {
        EnemyBattlefield.Add(BattlefieldCard);
        NewIndex = EnemyBattlefield.Num() - 1;

        UE_LOG(LogTemp, Log, TEXT("[CombatManager] Enemy summoned %s (%d ATK/%d HP) at index %d"),
            *CreatureCard.Name.ToString(), CreatureCard.Attack, CreatureCard.Health, NewIndex);
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

    Battlefield.RemoveAt(BattlefieldIndex);

    UE_LOG(LogTemp, Log, TEXT("[CombatManager] Removed card at index %d from %s battlefield"),
        BattlefieldIndex, bIsPlayerSide ? TEXT("player") : TEXT("enemy"));

    // Fire remove event for Blueprints
    OnCreatureRemoved.Broadcast(BattlefieldIndex, bIsPlayerSide);
}


bool ACombatManager::DamageSpecificBattlefieldCard(int32 BattlefieldIndex, bool bIsPlayerSide, int32 Damage)
{
    TArray<FBattlefieldCard>& Battlefield = bIsPlayerSide ? PlayerBattlefield : EnemyBattlefield;

    if (!Battlefield.IsValidIndex(BattlefieldIndex) || Damage <= 0)
    {
        return false;
    }

    FBattlefieldCard& Card = Battlefield[BattlefieldIndex];
    Card.CurrentHealth = FMath::Max(0, Card.CurrentHealth - Damage);

    UE_LOG(LogTemp, Log, TEXT("[CombatManager] %s takes %d damage, health now %d"),
        *Card.CardData.Name.ToString(), Damage, Card.CurrentHealth);

    // Remove card if health reaches 0
    if (Card.CurrentHealth <= 0)
    {
        UE_LOG(LogTemp, Log, TEXT("[CombatManager] %s destroyed"), *Card.CardData.Name.ToString());
        RemoveCardFromBattlefield(BattlefieldIndex, bIsPlayerSide);
    }

    OnCardDamaged.Broadcast(BattlefieldIndex, Damage, bIsPlayerSide);


    return true;
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

bool ACombatManager::DamageFirstAvailableEnemyCard(int32 Damage)
{
    for (int32 i = 0; i < EnemyBattlefield.Num(); i++)
    {
        if (EnemyBattlefield[i].CurrentHealth > 0)
        {
            return DamageSpecificBattlefieldCard(i, false, Damage);
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

    // Simple AI: Enemy attacks Player Health or creatures
    int32 EnemyDamage = FMath::RandRange(10, 20);
    DamagePlayer(EnemyDamage);

    // Check if combat should end
    CheckWinConditions();

    // If combat is still active, start player turn
    if (IsCombatActive())
    {
        FTimerHandle PlayerTurnTimer;
        GetWorld()->GetTimerManager().SetTimer(PlayerTurnTimer, [this]()
            {
                SetCombatState(ECombatState::PlayerTurn);
            }, 1.0f, false);
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