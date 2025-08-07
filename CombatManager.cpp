// CombatManager.cpp
#include "CombatManager.h"
#include "HandManager.h"
#include "Blueprint/UserWidget.h"
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

void ACombatManager::StartCombat(const FEnemyData& Enemy, const TArray<int32>& PlayerDeckIDs)
{
    if (!HandManager)
    {
        UE_LOG(LogTemp, Error, TEXT("[CombatManager] No HandManager assigned!"));
        return;
    }

    // Set up enemy
    CurrentEnemy = Enemy;

    // Set up player deck and draw starting hand
    HandManager->SetPlayerDeck(PlayerDeckIDs);
    HandManager->DrawStartingHand();

    // Bind to hand manager events
    if (!HandManager->OnCardPlayed.IsBound())
    {
        HandManager->OnCardPlayed.AddDynamic(this, &ACombatManager::OnCardPlayed);
    }

    // Create combat UI
    if (CombatUIClass && !CombatUI)
    {
        CombatUI = CreateWidget<UUserWidget>(GetWorld(), CombatUIClass);
        if (CombatUI)
        {
            CombatUI->AddToViewport();
        }
    }

    if (CombatUI && HandManager) {
        if (UCombatUIWidget* TypedCombatUI = Cast<UCombatUIWidget>(CombatUI))
        {
            TypedCombatUI->InitializeUI(this, HandManager);
        }
    }

    // Set starting energy based on faction
    CurrentEnergy = MaxEnergyPerTurn;

    // Start combat
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
        // Draw new hand (typically 5 cards, but this could be configurable)
        HandManager->DrawCards(HandManager->StartingHandSize);
    }

    // Reset energy for next turn  
    CurrentEnergy = MaxEnergyPerTurn;

    SetCombatState(ECombatState::EnemyTurn);

    // Process enemy turn after a brief delay
    FTimerHandle EnemyTurnTimer;
    GetWorld()->GetTimerManager().SetTimer(EnemyTurnTimer, this, &ACombatManager::ProcessEnemyTurn, 1.5f, false);
}

void ACombatManager::DamagePlayer(int32 Damage)
{
    // In your game, damage goes to the Player Health, not a "player health"
    DamagePlayerHealth(Damage);
}

void ACombatManager::DamageEnemy(int32 Damage)
{
    if (Damage <= 0) return;

    CurrentEnemy.Health = FMath::Max(0, CurrentEnemy.Health - Damage);
    OnHealthChanged.Broadcast(false, CurrentEnemy.Health); // false = is enemy

    UE_LOG(LogTemp, Log, TEXT("[CombatManager] %s takes %d damage, health now %d"),
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
    DamagePlayerHealth(EnemyDamage);

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
    if (PlayedCard.CardType == ECardType::Creature)
    {
        // Creatures go to the battlefield (you'll need to implement battlefield management)
        UE_LOG(LogTemp, Log, TEXT("[CombatManager] Creature '%s' summoned to battlefield"), *PlayedCard.Name.ToString());
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