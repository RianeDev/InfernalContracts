// Fill out your copyright notice in the Description page of Project Settings.


#include "KCKGameplayLibrary.h"
#include "CardTypesHost.h"
#include "GameFramework/Actor.h"


void UKCKGameplayLibrary::ExecuteAbilityByID(int32 AbilityID, UDataTable* AbilityTable, UDataTable* CardTable, FCardData& CardData, AActor* Caster, AActor* Target)
{
    if (!AbilityTable || !Caster) return;

    const FCardAbility* Ability = nullptr;

    for (const FName& Row : AbilityTable->GetRowNames())
    {
        Ability = AbilityTable->FindRow<FCardAbility>(Row, TEXT("ExecuteAbility"));
        if (Ability && Ability->ID == AbilityID)
        {
            Ability = Ability;
            break;
        }
    }

    if (!Ability)
    {
        UE_LOG(LogTemp, Warning, TEXT("[KCK] Ability ID %d not found."), AbilityID);
        return;
    }

    switch (Ability->AbilityType)
    {
    case ECardAbilityType::DrawSpecificCard:
        if (Ability->CardIDsToAffect.Num() > 0)
            DrawSpecificCard(CardTable, Ability->CardIDsToAffect[0], Ability->Count, Target);
        break;

    case ECardAbilityType::DrawMultipleCards:
        DrawMultipleCards(CardTable, Ability->CardIDsToAffect, Ability->Count, Target);
        break;

    case ECardAbilityType::BuffAllCreatures:
        BuffAllOwnedCreatures(Caster, Ability->Amount, Ability->Amount);
        break;

    case ECardAbilityType::ApplyDamage:
        ApplyDamage(Target, Ability->Amount);
        break;

    case ECardAbilityType::HealActor:
        HealActor(Target, Ability->Amount);
        break;

    case ECardAbilityType::BuffCard:
        BuffCard(CardData, Ability->Amount, Ability->Amount);
        break;

    default:
        UE_LOG(LogTemp, Warning, TEXT("[KCK] Unknown or None ability type."));
        break;
    }
}

void UKCKGameplayLibrary::DrawSpecificCard(UDataTable * CardTable, int32 CardID, int32 Count, AActor * Target)
{
    if (!CardTable || !Target || Count <= 0) return;

    for (const FName& Row : CardTable->GetRowNames())
    {
        const FCardData* Card = CardTable->FindRow<FCardData>(Row, TEXT("DrawSpecificCard"));

        if (Card && Card->ID == CardID)
        {
            for (int32 i = 0; i < Count; ++i)
            {
                UE_LOG(LogTemp, Log, TEXT("[KCK] Drew card '%s' (ID %d) for %s"), *Card->Name.ToString(), Card->ID, *Target->GetName());
            }
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[KCK] Card ID %d not found in CardTable."), CardID);
}

void UKCKGameplayLibrary::DrawMultipleCards(UDataTable * CardTable, const TArray<int32>&CardIDs, int32 CountPerCard, AActor * Target)
{
    if (!CardTable || !Target) return;

    for (int32 CardID : CardIDs)
    {
        DrawSpecificCard(CardTable, CardID, CountPerCard, Target);
    }
}

void UKCKGameplayLibrary::BuffCard(FCardData & Card, int32 AttackIncrease, int32 HealthIncrease)
{
    Card.Attack += AttackIncrease;
    Card.Health += HealthIncrease;

    UE_LOG(LogTemp, Log, TEXT("[KCK] Buffed card %s to %d ATK / %d HP"), *Card.Name.ToString(), Card.Attack, Card.Health);
}

void UKCKGameplayLibrary::BuffAllOwnedCreatures(AActor * Owner, int32 AtkBoost, int32 HpBoost)
{
    if (!Owner) return;

    UE_LOG(LogTemp, Log, TEXT("[KCK] All owned creatures of %s gain +%d ATK / +%d HP"),
        *Owner->GetName(), AtkBoost, HpBoost);

    // TODO: Hook into actual card manager to loop through Owner's hand/field cards
}

void UKCKGameplayLibrary::ApplyDamage(AActor * Target, int32 Amount)
{
    if (!Target || Amount <= 0) return;

    UE_LOG(LogTemp, Log, TEXT("[KCK] ApplyDamage: %s takes %d damage."), *Target->GetName(), Amount);
}

void UKCKGameplayLibrary::HealActor(AActor * Target, int32 Amount)
{
    if (!Target || Amount <= 0) return;

    UE_LOG(LogTemp, Log, TEXT("[KCK] HealActor: %s heals %d HP."), *Target->GetName(), Amount);
}