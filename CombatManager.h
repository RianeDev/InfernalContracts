#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CardTypesHost.h"
#include "CombatTypes.h"
#include "CombatManager.generated.h"

// Forward declarations to avoid circular dependencies
class AHandManager;
class UCombatUIWidget;

USTRUCT(BlueprintType)
struct FEnemyData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Name;

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

    // Energy per turn (3 by default, varies by faction)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 CurrentEnergy = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 MaxEnergyPerTurn = 3;

    // Current faction (affects energy type: Souls/Blight/Light)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECardFaction PlayerFaction = ECardFaction::Demon;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FEnemyData CurrentEnemy;

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

    // ==== BLUEPRINT CALLABLE FUNCTIONS FOR DEVELOPERS ====

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat")
    void SetCombatUI(class UUserWidget* InCombatUI);

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat", CallInEditor)
    void StartCombat(const FEnemyData& Enemy, const TArray<int32>& PlayerDeckIDs);

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat", CallInEditor)
    void EndCombat(bool bPlayerWon);

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat", CallInEditor)
    void EndPlayerTurn();

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat", CallInEditor)
    void DamagePlayer(int32 Damage);

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat", CallInEditor)
    void DamageEnemy(int32 Damage);

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat", CallInEditor)
    void HealPlayer(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat", CallInEditor)
    void DamagePlayerHealth(int32 Damage);

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat", CallInEditor)
    void HealPlayerHealth(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat", CallInEditor)
    void SetPlayerEnergy(int32 NewEnergy);

    UFUNCTION(BlueprintCallable, Category = "Infernal Contracts|Combat", CallInEditor)
    void SpendEnergy(int32 Cost);

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

private:
    void SetCombatState(ECombatState NewState);
    void ProcessEnemyTurn();
    void CheckWinConditions();

    UFUNCTION()
    void OnCardPlayed(const FCardData& PlayedCard);
};