#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CardTypesHost.h"
#include "EnemyAIComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyAIAttemptedPlay, const FCardData&, CardPlayed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyAITurnEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyHealthChanged, int32, NewHealth);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class KEVESCARDKIT_API UEnemyAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnemyAIComponent();

    UPROPERTY(BlueprintReadWrite, Category = "Enemy AI")
    int32 Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI")
    int32 MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI")
    int32 MaxEnergyPerTurn = 3;

    UPROPERTY(BlueprintAssignable, Category = "Enemy AI")
    FOnEnemyHealthChanged OnEnemyHealthChanged;

    UPROPERTY(BlueprintReadWrite, Category = "Enemy AI")
    class ACombatManager* CombatManager = nullptr;

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    int32 GetCurrentEnergy() const { return CurrentEnergy; }

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    int32 GetMaxEnergy() const { return MaxEnergyPerTurn; }

    // Initialize AI with deck card IDs
    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void InitializeEnemyAI(const TArray<int32>& DeckCardIDs);

    // Start the enemy turn, triggers incremental card plays every 2.5 seconds
    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void StartEnemyTurn();

    // Called on timer tick to process one step of enemy logic (try play one card)
    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void ProcessEnemyTurnStep();

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void ModifyEnemyHealth(int32 HealthDelta);

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    bool DamageFirstAvailableEnemyCard(int32 Damage);

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void ResetEnergy() { CurrentEnergy = MaxEnergyPerTurn; }

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void SetCurrentEnergy(int32 NewEnergy);

    // Try to play card by index from hand, returns true if successful
    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    bool TryPlayCard(int32 HandIndex);

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    int32 GetHandSize() const { return EnemyHand.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    FCardData GetCardInHand(int32 Index) const;

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void ClearHand();

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void ShuffleDiscardIntoDeck();

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void AddCardToDiscard(int32 CardID);

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void ShuffleDeck();

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void DrawCards(int32 Count);

    // Delegate when AI attempts to play a card
    UPROPERTY(BlueprintAssignable, Category = "Enemy AI")
    FOnEnemyAIAttemptedPlay OnEnemyAIAttemptedPlay;

    // Delegate when enemy turn ends
    UPROPERTY(BlueprintAssignable, Category = "Enemy AI")
    FOnEnemyAITurnEnded OnEnemyAITurnEnded;

protected:
    virtual void BeginPlay() override;

    UPROPERTY()
    TArray<FCardData> EnemyDeck;

    UPROPERTY()
    TArray<FCardData> EnemyHand;

    UPROPERTY()
    TArray<int32> DiscardPileCardIDs;

    UPROPERTY(BlueprintReadOnly, Category = "Enemy AI")
    int32 CurrentEnergy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI")
    UDataTable* CardDataTable;

    FCardData* FindCardByID(int32 CardID) const;

    // BlueprintNativeEvent so AI logic can be overridden in Blueprints
    UFUNCTION(BlueprintNativeEvent, Category = "Enemy AI")
    int32 SelectCardToPlay();

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void EndTurn();

public:
    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void SetCombatManager(ACombatManager* InCombatManager);

protected:
    // For incremental play logic
    int32 NextCardToPlayIndex = 0;

    FTimerHandle EnemyTurnStepTimerHandle;

    bool bIsEnemyTurnActive = false;
};
