#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CardTypesHost.h"
#include "CombatTypes.h"
#include "EnemyAIComponent.h"
#include "CombatManager.generated.h"

// Forward declarations to avoid circular dependencies
class AHandManager;
class UCombatUIWidget;

// Structure to represent a card on the battlefield
USTRUCT(BlueprintType)
struct FBattlefieldCard
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FCardData CardData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentHealth = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentAttack = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPlayerOwned = true;

    // Unique identifier that never changes (used for targeting)
    UPROPERTY(BlueprintReadOnly)
    int32 UniqueID = -1;

    // Current array index (can change when cards are removed)
    UPROPERTY(BlueprintReadOnly)
    int32 BattlefieldIndex = -1;

    // Optional reference to the visual actor (if using BattlefieldCardActor)
    UPROPERTY(BlueprintReadWrite)
    class ABattlefieldCardActor* VisualActor = nullptr;

    FBattlefieldCard()
    {
        CardData = FCardData();
        CurrentHealth = 0;
        CurrentAttack = 0;
        bIsPlayerOwned = true;
        UniqueID = -1;
        BattlefieldIndex = -1;
        VisualActor = nullptr;
    }

    FBattlefieldCard(const FCardData& InCardData, bool bPlayerOwned)
    {
        CardData = InCardData;
        CurrentHealth = InCardData.Health;
        CurrentAttack = InCardData.Attack;
        bIsPlayerOwned = bPlayerOwned;
        UniqueID = -1; // Will be set when added to battlefield
        BattlefieldIndex = -1;
        VisualActor = nullptr;
    }
};

USTRUCT(BlueprintType)
struct FEnemyData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* ActorReference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Health = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxHealth = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* EnemyArt = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> EnemyDeckCardIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText FlavorText;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatStateChanged, ECombatState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, bool, bIsPlayer, int32, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCreatureSummoned, const FBattlefieldCard&, Card, int32, Index, bool, bPlayerOwned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCreatureRemoved, int32, Index, bool, bPlayerOwned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCardDamaged, int32, BattlefieldIndex, int32, DamageAmount, bool, bIsPlayerSide);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCardDamagedByUniqueID, int32, UniqueID, int32, DamageAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCardBanishedSignature, const FCardData&, Card, bool, bWasSummoned, int32, UniqueID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCardDiscardedSignature, const FCardData&, Card, bool, bWasSummoned, int32, UniqueID);

UCLASS(BlueprintType, Blueprintable)
class KEVESCARDKIT_API ACombatManager : public AActor
{
    GENERATED_BODY()

public:
    ACombatManager();

protected:
    virtual void BeginPlay() override;

public:
    // Combat State
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombatState CurrentState = ECombatState::None;

    // Player Health Health (persistent across combats)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 PlayerHealth = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 PlayerMaxHealth = 20;

    // Player Energy per turn (3 by default, varies by faction)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 CurrentEnergy = 3;

    //Player energy
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 MaxEnergyPerTurn = 3;

    // Current faction (affects energy type: Souls/Blight/Light)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECardFaction PlayerFaction = ECardFaction::Demon;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FEnemyData CurrentEnemy;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* EnemyActor;

    // Battlefield - Cards currently in play
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    TArray<FBattlefieldCard> PlayerBattlefield;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    TArray<FBattlefieldCard> EnemyBattlefield;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    FBattlefieldCard CardChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat")
    FOnCardDamaged OnCardDamaged;

    UPROPERTY(BlueprintAssignable, Category = "Combat")
    FOnCardDamagedByUniqueID OnCardDamagedByUniqueID;

    UPROPERTY(BlueprintAssignable, Category = "Combat")
    FOnCardBanishedSignature OnCardBanished;

    UPROPERTY(BlueprintAssignable, Category = "Combat")
    FOnCardDiscardedSignature OnCardDiscarded;

    // References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    class AHandManager* HandManager;

    // UI References - Removed TSubclassOf to avoid circular dependency
    UPROPERTY(BlueprintReadOnly, Category = "UI")
    class UUserWidget* CombatUI;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCombatStateChanged OnCombatStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnCreatureSummoned OnCreatureSummoned;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnCreatureRemoved OnCreatureRemoved;

    // ==== BLUEPRINT CALLABLE FUNCTIONS FOR DEVELOPERS ====

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat")
    int32 GetEnemyCurrentEnergy() const
    {
        return EnemyAIComponent ? EnemyAIComponent->GetCurrentEnergy() : 0;
    }

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat")
    int32 GetEnemyMaxEnergy() const
    {
        return EnemyAIComponent ? EnemyAIComponent->GetMaxEnergy() : 0;
    }

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat")
    void SetCombatUI(class UUserWidget* InCombatUI);

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat", CallInEditor)
    void StartCombat(const FEnemyData& Enemy, const TArray<int32>& PlayerDeckIDs);

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat", CallInEditor)
    void EndCombat(bool bPlayerWon);

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat", CallInEditor)
    void EndPlayerTurn();

    // Enhanced damage functions that target battlefield cards first
    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat", CallInEditor)
    void ModifyPlayerHealth(int32 HealthDelta);

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat", CallInEditor)
    void SetPlayerEnergy(int32 NewEnergy);

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat", CallInEditor)
    void SpendEnergy(int32 Cost);

    // Battlefield management
    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat", CallInEditor)
    void SummonCreature(const FCardData& CreatureCard, bool bIsPlayerOwned = true);

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat", CallInEditor)
    void RemoveCardFromBattlefield(int32 BattlefieldIndex, bool bIsPlayerSide = true);

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat")
    void BanishCard(const FCardData& CardToBanish);

    // Searches battlefield cards by CardID
    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat")
    int32 FindUniqueIDByCardID(int32 CardID, bool bIsPlayerSide) const;

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat", CallInEditor)
    bool DamageSpecificBattlefieldCard(int32 BattlefieldIndex, bool bIsPlayerSide, int32 Damage);

    // NEW: Unique ID based functions
    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat")
    bool DamageBattlefieldCardByUniqueID(int32 UniqueID, int32 Damage);

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat")
    FBattlefieldCard GetBattlefieldCardByUniqueID(int32 UniqueID, bool& bFound) const;

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat")
    bool RemoveBattlefieldCardByUniqueID(int32 UniqueID);

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "Infernal Contracts|Combat")
    bool IsPlayerTurn() const { return CurrentState == ECombatState::PlayerTurn; }

    UFUNCTION(BlueprintPure, Category = "Infernal Contracts|Combat")
    bool IsCombatActive() const { return CurrentState == ECombatState::PlayerTurn || CurrentState == ECombatState::EnemyTurn; }

    UFUNCTION(BlueprintPure, Category = "Infernal Contracts|Combat")
    float GetPlayerHealthPercent() const { return PlayerMaxHealth > 0 ? (float)PlayerHealth / PlayerMaxHealth : 0.0f; }

    UFUNCTION(BlueprintPure, Category = "Infernal Contracts|Combat")
    bool CanAffordCard(int32 CardCost) const { return CurrentEnergy >= CardCost; }

    UFUNCTION(BlueprintPure, Category = "Infernal Contracts|Combat")
    FString GetEnergyTypeName() const
    {
        switch (PlayerFaction)
        {
        case ECardFaction::Demon: return TEXT("Souls");
        case ECardFaction::Undead: return TEXT("Blight");
        case ECardFaction::Angel: return TEXT("Light");
        default: return TEXT("Energy");
        }
    }

    UFUNCTION(BlueprintPure, Category = "Infernal Contracts|Combat")
    float GetEnemyHealthPercent() const { return CurrentEnemy.MaxHealth > 0 ? (float)CurrentEnemy.Health / CurrentEnemy.MaxHealth : 0.0f; }

    UFUNCTION(BlueprintPure, Category = "Infernal Contracts|Combat")
    bool HasPlayerCardsWithHealth() const;

    UFUNCTION(BlueprintPure, Category = "Infernal Contracts|Combat")
    bool HasEnemyCardsWithHealth() const;

    UFUNCTION(BlueprintPure, Category = "Infernal Contracts|Combat")
    TArray<FBattlefieldCard> GetPlayerBattlefield() const { return PlayerBattlefield; }

    UFUNCTION(BlueprintPure, Category = "Infernal Contracts|Combat")
    TArray<FBattlefieldCard> GetEnemyBattlefield() const { return EnemyBattlefield; }

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat")
    void OnCardPlayed(const FCardData& PlayedCard);

    // ENEMY AI

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat")
    bool PlayEnemyCard(const FCardData& CardToPlay);

    void HandleEnemyHealthChanged(int32 NewHealth);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
    UEnemyAIComponent* EnemyAIComponent;

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat")
    void DamageEnemy(int32 Damage);

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat")
    void SetCombatState(ECombatState NewState);

    void ProcessEnemyTurn();
    void OnEnemyTurnComplete();
    void CheckWinConditions();
    void BroadcastBattlefieldUpdate(bool bPlayerSideChanged = true, const FBattlefieldCard* CardChanged = nullptr);

    // Enhanced damage targeting functions
    bool DamageFirstAvailablePlayerCard(int32 Damage);

    bool DamageFirstAvailableEnemyCard(int32 Damage);

    // Unique ID management
    static int32 NextUniqueCardID;

    void UpdateBattlefieldIndices();

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat")
    int32 FindBattlefieldIndexByUniqueID(int32 UniqueID, bool bIsPlayerSide) const;
};