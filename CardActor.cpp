// Fill out your copyright notice in the Description page of Project Settings.


#include "CardActor.h"
#include "KCKGameplayLibrary.h"


ACardActor::ACardActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ACardActor::BeginPlay()
{
    Super::BeginPlay();
}

void ACardActor::InitializeCard(const FCardData& InData, UDataTable* InCardTable, UDataTable* InAbilityTable)
{
    CardData = InData;
    CardTable = InCardTable;
    AbilityTable = InAbilityTable;
}

void ACardActor::ActivateAbility(AActor* Target)
{
    int32 AbilityID = (AbilityIDOverride != -1) ? AbilityIDOverride : CardData.AbilityID;

    if (AbilityID >= 0 && AbilityTable && CardTable)
    {
        UKCKGameplayLibrary::ExecuteAbilityByID(AbilityID, AbilityTable, CardTable, CardData, this, Target);
    }
}