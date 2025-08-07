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

FCardDisplayData UCombatUIWidget::GetCardDisplayDataAtIndex(int32 HandIndex) const
{
    FCardData CardData = GetCardDataAtIndex(HandIndex);
    bool bIsPlayable = CanPlayCardAtIndex(HandIndex) && IsPlayerTurn();

    return CreateCardDisplayData(CardData, HandIndex, bIsPlayable);
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

FString UCombatUIWidget::GetCardTypeDisplayName(ECardType CardType) const
{
    switch (CardType)
    {
    case ECardType::Creature: return TEXT("Creature");
    case ECardType::Spell: return TEXT("Spell");
    case ECardType::Power: return TEXT("Power");
    case ECardType::Skill: return TEXT("Skill");
    case ECardType::Champion: return TEXT("Champion");
    default: return TEXT("Unknown");
    }
}

FString UCombatUIWidget::GetFactionDisplayName(ECardFaction Faction) const
{
    switch (Faction)
    {
    case ECardFaction::Demon: return TEXT("Demon");
    case ECardFaction::Undead: return TEXT("Undead");
    case ECardFaction::Angel: return TEXT("Angel");
    default: return TEXT("Neutral");
    }
}

FLinearColor UCombatUIWidget::GetFactionColor(ECardFaction Faction) const
{
    switch (Faction)
    {
    case ECardFaction::Demon:
        return FLinearColor::Red;
    case ECardFaction::Undead:
        return FLinearColor(0.4f, 0.2f, 0.4f, 1.0f); // Purple-ish
    case ECardFaction::Angel:
        return FLinearColor(1.0f, 1.0f, 0.8f, 1.0f); // Light yellow
    default:
        return FLinearColor::White;
    }
}

bool UCombatUIWidget::ShouldShowAttackHealth(ECardType CardType) const
{
    return CardType == ECardType::Creature || CardType == ECardType::Champion;
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
        // Create array of FCardDisplayData from current hand
        TArray<FCardDisplayData> DisplayDataArray;

        for (int32 i = 0; i < HandManager->GetHandSize(); i++)
        {
            FCardData CardData = HandManager->GetCardInHand(i);
            bool bIsPlayable = CanPlayCardAtIndex(i) && IsPlayerTurn();

            FCardDisplayData DisplayData = CreateCardDisplayData(CardData, i, bIsPlayable);
            DisplayDataArray.Add(DisplayData);
        }

        // Broadcast the array of FCardDisplayData instead of FCardData
        OnUIHandChanged.Broadcast(
            DisplayDataArray,
            HandManager->GetHandSize(),
            HandManager->GetDeckSize()
        );
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
            FCardDisplayData DisplayData = CreateCardDisplayData(CardData, i, bCanPlay);
            OnUICardSlotChanged.Broadcast(i, DisplayData, true, bCanPlay);
        }
        else
        {
            // Empty card slot - broadcast empty data so developer can hide slot
            FCardDisplayData EmptyDisplayData;
            OnUICardSlotChanged.Broadcast(i, EmptyDisplayData, false, false);
        }
    }
}

FCardDisplayData UCombatUIWidget::CreateCardDisplayData(const FCardData& CardData, int32 HandIndex, bool bIsPlayable) const
{
    FCardDisplayData DisplayData;

    // Core Card Data
    DisplayData.CardData = CardData;
    DisplayData.HandIndex = HandIndex;

    // Playability
    DisplayData.bIsPlayable = bIsPlayable;
    DisplayData.bIsHovered = false;
    DisplayData.bIsSelected = false;

    // Display Information
    DisplayData.CardTypeDisplayName = GetCardTypeDisplayName(CardData.CardType);
    DisplayData.FactionDisplayName = GetFactionDisplayName(CardData.CardFaction);
    DisplayData.FactionColor = GetFactionColor(CardData.CardFaction);
    DisplayData.bShouldShowAttackHealth = ShouldShowAttackHealth(CardData.CardType);

    // Set border color and opacity based on playability
    if (!DisplayData.bIsPlayable)
    {
        DisplayData.CardBorderColor = FLinearColor::Gray;
        DisplayData.DisplayOpacity = 0.6f;
    }
    else
    {
        DisplayData.CardBorderColor = DisplayData.FactionColor;
        DisplayData.DisplayOpacity = 1.0f;
    }

    // Format text fields for convenience
    DisplayData.FormattedCostText = FString::Printf(TEXT("%d"), CardData.Cost);

    if (DisplayData.bShouldShowAttackHealth)
    {
        DisplayData.FormattedAttackText = FString::Printf(TEXT("%d"), CardData.Attack);
        DisplayData.FormattedHealthText = FString::Printf(TEXT("%d"), CardData.Health);
    }
    else
    {
        DisplayData.FormattedAttackText = TEXT("");
        DisplayData.FormattedHealthText = TEXT("");
    }

    return DisplayData;
}