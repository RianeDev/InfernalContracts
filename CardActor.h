// CardActor.h - Card Actor for executing abilities
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CardTypesHost.h"
#include "Engine/DataTable.h"
#include "CardActor.generated.h"

UCLASS(BlueprintType, Blueprintable)
class KEVESCARDKIT_API ACardActor : public AActor
{
    GENERATED_BODY()

public:
    ACardActor();

protected:
    virtual void BeginPlay() override;

public:
    // Card data this actor represents
    UPROPERTY(BlueprintReadOnly, Category = "Card")
    FCardData CardData;

    // Data table references
    UPROPERTY(BlueprintReadWrite, Category = "Card")
    UDataTable* CardDataTable;

    UPROPERTY(BlueprintReadWrite, Category = "Card")
    UDataTable* AbilityDataTable;

    // Initialize the card actor with data
    UFUNCTION(BlueprintCallable, Category = "Card")
    void InitializeCard(const FCardData& InCardData, UDataTable* InCardDataTable, UDataTable* InAbilityDataTable);

    // Execute the card's ability
    UFUNCTION(BlueprintCallable, Category = "Card")
    void ActivateAbility(AActor* Target = nullptr);

    // Get card data
    UFUNCTION(BlueprintPure, Category = "Card")
    FCardData GetCardData() const { return CardData; }

private:
    // Internal function to execute ability by ID
    void ExecuteAbilityByID(int32 AbilityID, AActor* Target);

    // Helper function to find ability data
    FCardAbility* FindAbilityByID(int32 AbilityID);
};