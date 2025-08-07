// CombatUIWidget.cpp
#include "CombatUIWidget.h"

UCombatUIWidget::UCombatUIWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    CombatManager = nullptr;
    HandManager = nullptr;
}

void UCombatUIWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UCombatUIWidget::NativeDestruct()
{
    UnbindFromManagers();
    Super::NativeDestruct();
}

void UCombatUIWidget::InitializeUI(ACombatManager* InCombatManager, AHandManager* InHandManager)
{
    // Unbind from previous managers if any
    UnbindFromManagers();

    CombatManager = InCombatManager;
    HandManager = InHandManager;

    // Bind to new managers
    BindToManagers();

    // Initial UI refresh - broadcast all current data
    RefreshAllUI();

    UE_LOG(LogTemp, Log, TEXT("[CombatUI] Initialized with CombatManager and HandManager"));
}

void UCombatUIWidget::RefreshAllUI()
{
    BroadcastHealthUpdate();
    BroadcastEnergyUpdate();
    BroadcastEnemyUpdate();
    BroadcastHandUpdate();

    if (CombatManager)
    {
        OnManagerCombatStateChanged(CombatManager->CurrentState);
    }

    CheckCardPlayability();
}

void UCombatUIWidget::RequestEndTurn()
{
    if (CombatManager && IsPlayerTurn())
    {
        CombatManager->EndPlayerTurn();
        UE_LOG(LogTemp, Log, TEXT("[CombatUI] Player requested end turn"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatUI] Cannot end turn - not player's turn or no combat manager"));
    }
}

void UCombatUIWidget::RequestPlayCard(int32 HandIndex, AActor* Target)
{
    if (!HandManager || !CombatManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatUI] Cannot play card - missing managers"));
        return;
    }

    if (!IsPlayerTurn())
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatUI] Cannot play card - not player turn"));
        return;
    }

    if (!CanPlayCardAtIndex(HandIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatUI] Cannot play card at index %d - insufficient energy or invalid card"), HandIndex);
        return;
    }

    // Play the card
    if (HandManager->PlayCard(HandIndex, Target))
    {
        UE_LOG(LogTemp, Log, TEXT("[CombatUI] Successfully requested play card at index %d"), HandIndex);
    }
}

bool UCombatUIWidget::CanPlayCardAtIndex(int32 HandIndex) const
{
    if (!HandManager || !CombatManager) return false;

    return HandManager->CanPlayCard(HandIndex, CombatManager->CurrentEnergy);
}

bool UCombatUIWidget::IsPlayerTurn() const
{
    return CombatManager ? CombatManager->IsPlayerTurn() : false;
}

FCardData UCombatUIWidget::GetCardDataAtIndex(int32 HandIndex) const
{
    if (HandManager)
    {
        return HandManager->GetCardInHand(HandIndex);
    }
    return FCardData(); // Return empty if invalid
}

float UCombatUIWidget::GetHealthPercent() const
{
    return CombatManager ? CombatManager->GetPlayerHealthPercent() : 0.0f;
}

float UCombatUIWidget::GetEnemyHealthPercent() const
{
    return CombatManager ? CombatManager->GetEnemyHealthPercent() : 0.0f;
}

FString UCombatUIWidget::GetCombatStateDisplayText(ECombatState State) const
{
    switch (State)
    {
    case ECombatState::Starting:
        return TEXT("Combat Starting...");
    case ECombatState::PlayerTurn:
        return TEXT("Your Turn");
    case ECombatState::EnemyTurn:
        return TEXT("Enemy Turn");
    case ECombatState::Victory:
        return TEXT("Victory!");
    case ECombatState::Defeat:
        return TEXT("Defeated...");
    default:
        return TEXT("...");
    }
}

void UCombatUIWidget::OnManagerCombatStateChanged(ECombatState NewState)
{
    FString StateText = GetCombatStateDisplayText(NewState);
    OnUICombatStateChanged.Broadcast(NewState, StateText);

    // Check card playability when combat state changes
    CheckCardPlayability();
}

void UCombatUIWidget::OnManagerHealthChanged(bool bIsPlayer, int32 NewHealth)
{
    if (bIsPlayer)
    {
        BroadcastHealthUpdate();
    }
    else
    {
        BroadcastEnemyUpdate();
    }
}

void UCombatUIWidget::OnManagerHandUpdated(const TArray<FCardData>& CurrentHand, int32 HandSize)
{
    BroadcastHandUpdate();
    CheckCardPlayability();
}

void UCombatUIWidget::OnManagerCardPlayed(const FCardData& PlayedCard)
{
    // Energy changed after playing card
    BroadcastEnergyUpdate();
    CheckCardPlayability();

    UE_LOG(LogTemp, Log, TEXT("[CombatUI] Processed card played: %s"), *PlayedCard.Name.ToString());
}

void UCombatUIWidget::BindToManagers()
{
    if (CombatManager)
    {
        CombatManager->OnCombatStateChanged.AddDynamic(this, &UCombatUIWidget::OnManagerCombatStateChanged);
        CombatManager->OnHealthChanged.AddDynamic(this, &UCombatUIWidget::OnManagerHealthChanged);
    }

    if (HandManager)
    {
        HandManager->OnHandUpdated.AddDynamic(this, &UCombatUIWidget::OnManagerHandUpdated);
        HandManager->OnCardPlayed.AddDynamic(this, &UCombatUIWidget::OnManagerCardPlayed);
    }
}

void UCombatUIWidget::UnbindFromManagers()
{
    if (CombatManager)
    {
        CombatManager->OnCombatStateChanged.RemoveDynamic(this, &UCombatUIWidget::OnManagerCombatStateChanged);
        CombatManager->OnHealthChanged.RemoveDynamic(this, &UCombatUIWidget::OnManagerHealthChanged);
    }

    if (HandManager)
    {
        HandManager->OnHandUpdated.RemoveDynamic(this, &UCombatUIWidget::OnManagerHandUpdated);
        HandManager->OnCardPlayed.RemoveDynamic(this, &UCombatUIWidget::OnManagerCardPlayed);
    }
}

void UCombatUIWidget::BroadcastHealthUpdate()
{
    if (CombatManager)
    {
        OnUIHealthChanged.Broadcast(
            CombatManager->PlayerHealth,
            CombatManager->PlayerMaxHealth
        );
    }
}

void UCombatUIWidget::BroadcastEnergyUpdate()
{
    if (CombatManager)
    {
        OnUIEnergyChanged.Broadcast(
            CombatManager->CurrentEnergy,
            CombatManager->MaxEnergyPerTurn,
            CombatManager->GetEnergyTypeName()
        );
    }
}

void UCombatUIWidget::BroadcastEnemyUpdate()
{
    if (CombatManager)
    {
        const FEnemyData& Enemy = CombatManager->CurrentEnemy;
        OnUIEnemyChanged.Broadcast(
            Enemy.Name,
            Enemy.Health,
            Enemy.MaxHealth,
            Enemy.EnemyArt
        );
    }
}

void UCombatUIWidget::BroadcastHandUpdate()
{
    if (HandManager)
    {
        OnUIHandChanged.Broadcast(
            HandManager->CurrentHand,
            HandManager->GetHandSize(),
            HandManager->GetDeckSize()
        );

        // Also check card playability whenever hand updates
        CheckCardPlayability();
    }
}

void UCombatUIWidget::CheckCardPlayability()
{
    if (!HandManager || !CombatManager) return;

    // Broadcast data for all possible hand slots (up to max hand size)
    // This lets developers show/hide card slots based on validity
    for (int32 i = 0; i < HandManager->MaxHandSize; i++)
    {
        if (i < HandManager->GetHandSize())
        {
            // Valid card slot - has card data
            FCardData CardData = HandManager->GetCardInHand(i);
            bool bCanPlay = CanPlayCardAtIndex(i) && IsPlayerTurn();
            OnUICardSlotChanged.Broadcast(i, CardData, true, bCanPlay);
        }
        else
        {
            // Empty card slot - broadcast empty data so developer can hide slot
            FCardData EmptyCard;
            OnUICardSlotChanged.Broadcast(i, EmptyCard, false, false);
        }
    }
}