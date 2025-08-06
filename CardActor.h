// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CardTypesHost.h"
#include "CardActor.generated.h"

UCLASS()
class KEVESCARDKIT_API ACardActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACardActor();
	
    // Called when the game starts or spawned
    virtual void BeginPlay() override;

    // Card Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card")
    FCardData CardData;

    // Optional: Ability ID if you want to override CardData's
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card")
    int32 AbilityIDOverride = -1;

    // Optional: Ability table
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card")
    UDataTable* AbilityTable;

    // Optional: Card table (if we ever want card->card interaction)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card")
    UDataTable* CardTable;

    // Use ability
    UFUNCTION(BlueprintCallable, Category = "Card")
    void ActivateAbility(AActor* Target);

    // Useful when first populating
    UFUNCTION(BlueprintCallable, Category = "Card")
    void InitializeCard(const FCardData& InData, UDataTable* InCardTable, UDataTable* InAbilityTable);

	
};
