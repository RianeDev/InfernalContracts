// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "CardTypesHost.h"
#include "KCKGameplayLibrary.generated.h"

/**
 * 
 */
UCLASS()
class KEVESCARDKIT_API UKCKGameplayLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

    static void ExecuteAbilityByID(
        int32 AbilityID,
        UDataTable* AbilityTable,
        UDataTable* CardTable,
        FCardData& CardData,
        AActor* Caster,
        AActor* Target
    );

    UFUNCTION(BlueprintCallable, Category = "KCK|Abilities")
    static void DrawSpecificCard(UDataTable* CardTable, int32 CardID, int32 Count, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "KCK|Abilities")
    static void DrawMultipleCards(UDataTable* CardTable, const TArray<int32>& CardIDs, int32 CountPerCard, AActor* Target);

    // Buffs and damage
    UFUNCTION(BlueprintCallable, Category = "KCK|Abilities")
    static void BuffCard(FCardData& Card, int32 AttackIncrease, int32 HealthIncrease);

    UFUNCTION(BlueprintCallable, Category = "KCK|Abilities")
    static void BuffAllOwnedCreatures(AActor* Owner, int32 AtkBoost, int32 HpBoost);

    UFUNCTION(BlueprintCallable, Category = "KCK|Abilities")
    static void ApplyDamage(AActor* Target, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "KCK|Abilities")
    static void HealActor(AActor* Target, int32 Amount);
	
	
};
