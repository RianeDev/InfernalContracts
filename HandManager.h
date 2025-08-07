// HandManager.h - Enhanced Version
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CardTypesHost.h"
#include "CardActor.h"
#include "Components/Widget.h"
#include "Components/PanelWidget.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "HandManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHandUpdated, const TArray<FCardData>&, CurrentHand, int32, HandSize);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardPlayed, const FCardData&, PlayedCard);

UCLASS(BlueprintType, Blueprintable)
class KEVESCARDKIT_API AHandManager : public AActor
{
    GENERATED_BODY()

public:
    AHandManager();

protected:
    virtual void BeginPlay() override;

public:
    // Core Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card System")
    TSubclassOf<ACardActor> CardActorClass;

    UPROPERTY(BlueprintReadOnly, Category = "Card System")
    TArray<FCardData> CurrentHand;

    UPROPERTY(BlueprintReadOnly, Category = "Card System")
    TArray<FCardData> PlayerDeck;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card System")
    UDataTable* CardDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card System")
    UDataTable* AbilityDataTable;

    // Hand Management Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Settings")
    int32 MaxHandSize = 7;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Settings")
    int32 StartingHandSize = 5;

    // UI Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> CardWidgetClass;

    UPROPERTY(BlueprintReadOnly, Category = "UI")
    TArray<UUserWidget*> HandWidgets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    class UPanelWidget* HandPanel; // Reference to UI panel that holds cards

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnHandUpdated OnHandUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCardPlayed OnCardPlayed;

    // ==== BLUEPRINT CALLABLE FUNCTIONS FOR DEVELOPERS ====

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

    // UI Management
    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|UI", CallInEditor)
    void RefreshHandUI();

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|UI", CallInEditor)
    void SetHandUIPanel(UPanelWidget* Panel);

    // Card Playing
    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Gameplay", CallInEditor)
    bool PlayCard(int32 HandIndex, AActor* Target = nullptr);

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "Infernal Contracts|Info")
    int32 GetHandSize() const { return CurrentHand.Num(); }

    UFUNCTION(BlueprintPure, Category = "Infernal Contracts|Info")
    int32 GetDeckSize() const { return PlayerDeck.Num(); }

    UFUNCTION(BlueprintPure, Category = "Infernal Contracts|Info")
    FCardData GetCardInHand(int32 Index) const;

    UFUNCTION(BlueprintPure, Category = "Infernal Contracts|Info")
    bool CanPlayCard(int32 HandIndex, int32 CurrentMana = 0) const;

private:
    // Internal helper functions
    FCardData* FindCardByID(int32 CardID);
    void CreateCardWidget(const FCardData& CardData, int32 HandIndex);
    void DestroyCardWidget(int32 HandIndex);
    void UpdateCardWidget(int32 HandIndex);
};