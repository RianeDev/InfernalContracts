// Fill out your copyright notice in the Description page of Project Settings.


#include "HandManager.h"
#include "CardActor.h"
#include "Engine/World.h"


AHandManager::AHandManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AHandManager::BeginPlay()
{
    Super::BeginPlay();
}

void AHandManager::AddCardByID(int32 CardID, int32 IndexInHand)
{
    if (!CardDataTable) return;

    for (const FName& RowName : CardDataTable->GetRowNames())
    {
        if (const FCardData* Found = CardDataTable->FindRow<FCardData>(RowName, TEXT("SpawnCard")))
        {
            // logic to add to hand
        }
    }
}

void AHandManager::RemoveCardByID(int32 CardIndex, int32 IndexInHand)
{
    if (!CardDataTable) return;

    for (const FName& RowName : CardDataTable->GetRowNames())
    {
        if (const FCardData* Found = CardDataTable->FindRow<FCardData>(RowName, TEXT("SpawnCard")))
        {
            // logic to remove from hand
        }
    }
}