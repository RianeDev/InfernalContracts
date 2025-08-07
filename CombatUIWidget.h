// CombatUIWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CardTypesHost.h"
#include "HandManager.h"
#include "CombatManager.h"
#include "CombatUIWidget.generated.h"

// UI Event Delegates - Developers can bind to these however they want
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUIHealthChanged, int32, CurrentHealth, int32, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnUIEnergyChanged, int32, CurrentEnergy, int32, MaxEnergy, FString, EnergyTypeName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnUIEnemyChanged, FText, EnemyName, int32, EnemyHealth, int32, EnemyMaxHealth, UTexture2D*, EnemyPortrait);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnUIHandChanged, const TArray<FCardData>&, CurrentHand, int32, HandSize, int32, DeckSize);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUICombatStateChanged, ECombatState, NewState, FString, StateDisplayText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnUICardSlotChanged, int32, SlotIndex, const FCardData&, CardData, bool, bHasCard, bool, bCanPlay);

UCLASS(BlueprintType, Blueprintable)
class KEVESCARDKIT_API UCombatUIWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UCombatUIWidget(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

public:
    // === MANAGER REFERENCES ===

    UPROPERTY(BlueprintReadWrite, Category = "Combat UI")
    class ACombatManager* CombatManager;

    UPROPERTY(BlueprintReadWrite, Category = "Combat UI")
    class AHandManager* HandManager;

    // === UI EVENT DELEGATES - Bind to these in Blueprint ===

    UPROPERTY(BlueprintAssignable, Category = "UI Events")
    FOnUIHealthChanged OnUIHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "UI Events")
    FOnUIEnergyChanged OnUIEnergyChanged;

    UPROPERTY(BlueprintAssignable, Category = "UI Events")
    FOnUIEnemyChanged OnUIEnemyChanged;

    UPROPERTY(BlueprintAssignable, Category = "UI Events")
    FOnUIHandChanged OnUIHandChanged;

    UPROPERTY(BlueprintAssignable, Category = "UI Events")
    FOnUICombatStateChanged OnUICombatStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "UI Events")
    FOnUICardSlotChanged OnUICardSlotChanged;

    // === BLUEPRINT CALLABLE FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Combat UI")
    void InitializeUI(ACombatManager* InCombatManager, AHandManager* InHandManager);

    UFUNCTION(BlueprintCallable, Category = "Combat UI")
    void RefreshAllUI();

    // Player Actions - Call these from your UI buttons/interactions
    UFUNCTION(BlueprintCallable, Category = "Combat UI")
    void RequestEndTurn();

    UFUNCTION(BlueprintCallable, Category = "Combat UI")
    void RequestPlayCard(int32 HandIndex, AActor* Target = nullptr);

    // Utility Functions for Developers
    UFUNCTION(BlueprintPure, Category = "Combat UI")
    bool CanPlayCardAtIndex(int32 HandIndex) const;

    UFUNCTION(BlueprintPure, Category = "Combat UI")
    bool IsPlayerTurn() const;

    UFUNCTION(BlueprintPure, Category = "Combat UI")
    FCardData GetCardDataAtIndex(int32 HandIndex) const;

    UFUNCTION(BlueprintPure, Category = "Combat UI")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Combat UI")
    float GetEnemyHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Combat UI")
    FString GetCombatStateDisplayText(ECombatState State) const;

protected:
    // === EVENT HANDLERS - Listen to manager events and convert to UI events ===

    UFUNCTION()
    void OnManagerCombatStateChanged(ECombatState NewState);

    UFUNCTION()
    void OnManagerHealthChanged(bool bIsPlayer, int32 NewHealth);

    UFUNCTION()
    void OnManagerHandUpdated(const TArray<FCardData>& CurrentHand, int32 HandSize);

    UFUNCTION()
    void OnManagerCardPlayed(const FCardData& PlayedCard);

private:
    // === HELPER FUNCTIONS ===

    void BindToManagers();
    void UnbindFromManagers();
    void BroadcastHealthUpdate();
    void BroadcastEnergyUpdate();
    void BroadcastEnemyUpdate();
    void BroadcastHandUpdate();
    void CheckCardPlayability();
};