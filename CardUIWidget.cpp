// CardUIWidget.cpp
#include "CardUIWidget.h"

UCardUIWidget::UCardUIWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    CurrentDisplayData.bIsPlayable = true;
    CurrentDisplayData.HandIndex = -1;
}

void UCardUIWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UCardUIWidget::InitializeCardWidget(const FCardData& CardData, int32 InHandIndex)
{
    CurrentDisplayData.CardData = CardData;
    CurrentDisplayData.HandIndex = InHandIndex;

    RefreshCardDisplayData();
}

void UCardUIWidget::UpdatePlayableState(bool bCanPlay)
{
    if (CurrentDisplayData.bIsPlayable != bCanPlay)
    {
        CurrentDisplayData.bIsPlayable = bCanPlay;
        RefreshCardDisplayData();

        // Broadcast specific playability event
        OnCardPlayabilityChanged.Broadcast(CurrentDisplayData.HandIndex, bCanPlay);
    }
}

void UCardUIWidget::UpdateHoverState(bool bIsHovered)
{
    if (CurrentDisplayData.bIsHovered != bIsHovered)
    {
        CurrentDisplayData.bIsHovered = bIsHovered;
        RefreshCardDisplayData();

        // Broadcast specific hover event
        OnCardHoverStateChanged.Broadcast(CurrentDisplayData.HandIndex, bIsHovered);
    }
}

void UCardUIWidget::UpdateSelectedState(bool bIsSelected)
{
    if (CurrentDisplayData.bIsSelected != bIsSelected)
    {
        CurrentDisplayData.bIsSelected = bIsSelected;
        RefreshCardDisplayData();
    }
}

void UCardUIWidget::RefreshCardDisplayData()
{
    FCardDisplayData NewData = BuildDisplayData();

    // Only broadcast if data actually changed
    if (HasDisplayDataChanged(NewData, LastDisplayData))
    {
        LastDisplayData = NewData;
        CurrentDisplayData = NewData;
        OnCardDisplayDataUpdated.Broadcast(NewData);
    }
}

void UCardUIWidget::RequestCardInteraction()
{
    if (CurrentDisplayData.bIsPlayable && CurrentDisplayData.HandIndex >= 0)
    {
        OnCardInteractionRequested.Broadcast(CurrentDisplayData.HandIndex, CurrentDisplayData.CardData);
        UE_LOG(LogTemp, Log, TEXT("[CardUI] Card interaction requested for: %s"), *CurrentDisplayData.CardData.Name.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[CardUI] Cannot interact with card - not playable or invalid index"));
    }
}

void UCardUIWidget::RequestCardHover()
{
    UpdateHoverState(true);
}

void UCardUIWidget::RequestCardUnhover()
{
    UpdateHoverState(false);
}

FString UCardUIWidget::GetCardTypeDisplayName(ECardType CardType) const
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

FString UCardUIWidget::GetFactionDisplayName(ECardFaction Faction) const
{
    switch (Faction)
    {
    case ECardFaction::Demon: return TEXT("Demon");
    case ECardFaction::Undead: return TEXT("Undead");
    case ECardFaction::Angel: return TEXT("Angel");
    default: return TEXT("Neutral");
    }
}

FLinearColor UCardUIWidget::GetFactionColor(ECardFaction Faction) const
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

bool UCardUIWidget::ShouldShowAttackHealth(ECardType CardType) const
{
    return CardType == ECardType::Creature || CardType == ECardType::Champion;
}

void UCardUIWidget::SetCardData_Implementation(const FCardData& CardData, int32 InHandIndex)
{
    InitializeCardWidget(CardData, InHandIndex);
}

void UCardUIWidget::UpdateCardDisplay_Implementation()
{
    RefreshCardDisplayData();
}

void UCardUIWidget::SetPlayable_Implementation(bool bCanPlay)
{
    UpdatePlayableState(bCanPlay);
}

FCardDisplayData UCardUIWidget::BuildDisplayData() const
{
    FCardDisplayData Data = CurrentDisplayData; // Copy current state

    // Update computed/derived values
    Data.CardTypeDisplayName = GetCardTypeDisplayName(Data.CardData.CardType);
    Data.FactionDisplayName = GetFactionDisplayName(Data.CardData.CardFaction);
    Data.FactionColor = GetFactionColor(Data.CardData.CardFaction);
    Data.bShouldShowAttackHealth = ShouldShowAttackHealth(Data.CardData.CardType);

    // Set border color based on playability and faction
    if (!Data.bIsPlayable)
    {
        Data.CardBorderColor = FLinearColor::Gray;
        Data.DisplayOpacity = 0.6f;
    }
    else if (Data.bIsSelected)
    {
        Data.CardBorderColor = FLinearColor::Yellow; // Selected state
        Data.DisplayOpacity = 1.0f;
    }
    else if (Data.bIsHovered)
    {
        Data.CardBorderColor = FLinearColor::White; // Hover state
        Data.DisplayOpacity = 1.0f;
    }
    else
    {
        Data.CardBorderColor = Data.FactionColor;
        Data.DisplayOpacity = 1.0f;
    }

    // Format text fields for convenience
    Data.FormattedCostText = FString::Printf(TEXT("%d"), Data.CardData.Cost);

    if (Data.bShouldShowAttackHealth)
    {
        Data.FormattedAttackText = FString::Printf(TEXT("%d"), Data.CardData.Attack);
        Data.FormattedHealthText = FString::Printf(TEXT("%d"), Data.CardData.Health);
    }
    else
    {
        Data.FormattedAttackText = TEXT("");
        Data.FormattedHealthText = TEXT("");
    }

    return Data;
}

bool UCardUIWidget::HasDisplayDataChanged(const FCardDisplayData& NewData, const FCardDisplayData& OldData) const
{
    // Check if any important display data has changed
    return (NewData.CardData.ID != OldData.CardData.ID ||
        NewData.HandIndex != OldData.HandIndex ||
        NewData.bIsPlayable != OldData.bIsPlayable ||
        NewData.bIsHovered != OldData.bIsHovered ||
        NewData.bIsSelected != OldData.bIsSelected ||
        NewData.CardData.Cost != OldData.CardData.Cost ||
        NewData.CardData.Attack != OldData.CardData.Attack ||
        NewData.CardData.Health != OldData.CardData.Health);
}