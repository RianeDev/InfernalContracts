// HandManager.h - Clean Data-Only Version
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CardTypesHost.h"
#include "CardActor.h"
#include "HandManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHandUpdated, const TArray<FCardData>&, CurrentHand, int32, HandSize);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardPlayed, const FCardData&, PlayedCard);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardAddedToHand, const FCardData&, AddedCard);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCardRemovedFromHand, const FCardData&, RemovedCard, int32, FormerIndex);

UCLASS(BlueprintType, Blueprintable)
class KEVESCARDKIT_API AHandManager : public AActor
{
    GENERATED_BODY()

public:
    AHandManager();

protected:
    virtual void BeginPlay() override;

public:
    // === CORE DATA ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card System")
    TSubclassOf<ACardActor> CardActorClass;

    UPROPERTY(BlueprintReadWrite, Category = "Card System")
    TArray<FCardData> CurrentHand;

    UPROPERTY(BlueprintReadWrite, Category = "Card System")
    TArray<FCardData> PlayerDeck;

    // A collection of the UniqueIDs of discarded cards.
    UPROPERTY(BlueprintReadWrite, Category = "Card System")
    TArray<int32> DiscardPileCardIDs;

    UPROPERTY(BlueprintReadWrite, Category = "Card System")
    TArray<int32> BanishedCardIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card System")
    UDataTable* CardDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card System")
    UDataTable* AbilityDataTable;

    // Combat Manager reference for damage dealing
    UPROPERTY(BlueprintReadWrite, Category = "Card System")
    class ACombatManager* CombatManager;

    // === SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Settings")
    int32 MaxHandSize = 7;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Settings")
    int32 StartingHandSize = 5;

    // === EVENTS ===
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnHandUpdated OnHandUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCardPlayed OnCardPlayed;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCardAddedToHand OnCardAddedToHand;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCardRemovedFromHand OnCardRemovedFromHand;

    // === BLUEPRINT CALLABLE FUNCTIONS ===

    // Core Hand Management
    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Hand", CallInEditor)
    bool AddCardToHand(int32 CardID);

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Hand", CallInEditor)
    bool RemoveCardFromHand(int32 HandIndex);

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Hand", CallInEditor)
    void ClearHand();

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Hand", CallInEditor)
    void DrawStartingHand();

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Hand", CallInEditor)
    void DrawCards(int32 Count = 1);

    // Deck Management
    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Deck", CallInEditor)
    void SetPlayerDeck(const TArray<int32>& CardIDs);

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Deck", CallInEditor)
    void AddCardToDeck(int32 CardID);

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Deck", CallInEditor)
    void ShuffleDeck();

    // Discard and Banish
    UFUNCTION(BlueprintCallable, Category = "Card System")
    void AddCardToDiscard(int32 CardID);

    UFUNCTION(BlueprintCallable, Category = "Card System")
    void ClearDiscardPile();

    UFUNCTION(BlueprintCallable, Category = "Card System")
    void ShuffleDiscardIntoDeck();

    UFUNCTION(BlueprintCallable, Category = "Card System")
    void RemoveCardFromAllPilesByCardID(int32 CardID);

    UFUNCTION(BlueprintCallable, Category = "Card System")
    void BanishCardByID(int32 CardID);

    // Card Playing
    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Gameplay", CallInEditor)
    bool PlayCard(int32 HandIndex, AActor* Target = nullptr);

    // Set Combat Manager reference
    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Gameplay")
    void SetCombatManager(class ACombatManager* InCombatManager);

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "Infernal Contracts|Info")
    int32 GetHandSize() const { return CurrentHand.Num(); }

    UFUNCTION(BlueprintPure, Category = "Infernal Contracts|Info")
    int32 GetDeckSize() const { return PlayerDeck.Num(); }

    UFUNCTION(BlueprintPure, Category = "Infernal Contracts|Info")
    FCardData GetCardInHand(int32 Index) const;

    UFUNCTION(BlueprintPure, Category = "Infernal Contracts|Info")
    bool CanPlayCard(int32 HandIndex, int32 CurrentEnergy = 0) const;

    UFUNCTION(BlueprintPure, Category = "Infernal Contracts|Info")
    bool IsHandFull() const { return CurrentHand.Num() >= MaxHandSize; }

    UFUNCTION(BlueprintPure, Category = "Infernal Contracts|Info")
    bool IsHandEmpty() const { return CurrentHand.Num() == 0; }

private:
    // Internal helper functions
    FCardData* FindCardByID(int32 CardID);
};
