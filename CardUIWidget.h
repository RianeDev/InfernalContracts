// CardUIWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ICardWidget.h"
#include "CardTypesHost.h"
#include "CardUIWidget.generated.h"

// Data structure for card UI updates
USTRUCT(BlueprintType)
struct FCardDisplayData
{
    GENERATED_BODY()

    // Core Card Data
    UPROPERTY(BlueprintReadOnly, Category = "Card Data")
    FCardData CardData;

    UPROPERTY(BlueprintReadOnly, Category = "Card Data")
    int32 HandIndex = -1;

    // Playability
    UPROPERTY(BlueprintReadOnly, Category = "Card State")
    bool bIsPlayable = true;

    UPROPERTY(BlueprintReadOnly, Category = "Card State")
    bool bIsHovered = false;

    UPROPERTY(BlueprintReadOnly, Category = "Card State")
    bool bIsSelected = false;

    // Display Information
    UPROPERTY(BlueprintReadOnly, Category = "Card Display")
    FString CardTypeDisplayName = TEXT("");

    UPROPERTY(BlueprintReadOnly, Category = "Card Display")
    FString FactionDisplayName = TEXT("");

    UPROPERTY(BlueprintReadOnly, Category = "Card Display")
    FLinearColor CardBorderColor = FLinearColor::White;

    UPROPERTY(BlueprintReadOnly, Category = "Card Display")
    FLinearColor FactionColor = FLinearColor::White;

    UPROPERTY(BlueprintReadOnly, Category = "Card Display")
    bool bShouldShowAttackHealth = false;

    UPROPERTY(BlueprintReadOnly, Category = "Card Display")
    float DisplayOpacity = 1.0f;

    // Formatted Text (for convenience)
    UPROPERTY(BlueprintReadOnly, Category = "Card Display")
    FString FormattedCostText = TEXT("0");

    UPROPERTY(BlueprintReadOnly, Category = "Card Display")
    FString FormattedAttackText = TEXT("0");

    UPROPERTY(BlueprintReadOnly, Category = "Card Display")
    FString FormattedHealthText = TEXT("0");
};

// Card UI Event Delegates - These are what developers will bind to
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardDisplayDataUpdated, const FCardDisplayData&, CardDisplayData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCardInteractionRequested, int32, HandIndex, const FCardData&, CardData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCardHoverStateChanged, int32, HandIndex, bool, bIsHovered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCardPlayabilityChanged, int32, HandIndex, bool, bCanPlay);

UCLASS(BlueprintType, Blueprintable)
class KEVESCARDKIT_API UCardUIWidget : public UUserWidget, public ICardWidget
{
    GENERATED_BODY()

public:
    UCardUIWidget(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void NativeConstruct() override;

public:
    // === CARD DATA ===

    UPROPERTY(BlueprintReadWrite, Category = "Card")
    FCardDisplayData CurrentDisplayData;

    // === UI DATA EVENTS - These are what developers bind to ===

    UPROPERTY(BlueprintAssignable, Category = "Card UI Events")
    FOnCardDisplayDataUpdated OnCardDisplayDataUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Card UI Events")
    FOnCardInteractionRequested OnCardInteractionRequested;

    UPROPERTY(BlueprintAssignable, Category = "Card UI Events")
    FOnCardHoverStateChanged OnCardHoverStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Card UI Events")
    FOnCardPlayabilityChanged OnCardPlayabilityChanged;

    // === BLUEPRINT CALLABLE FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Card UI")
    void InitializeCardWidget(const FCardData& CardData, int32 InHandIndex);

    UFUNCTION(BlueprintCallable, Category = "Card UI")
    void UpdatePlayableState(bool bCanPlay);

    UFUNCTION(BlueprintCallable, Category = "Card UI")
    void UpdateHoverState(bool bIsHovered);

    UFUNCTION(BlueprintCallable, Category = "Card UI")
    void UpdateSelectedState(bool bIsSelected);

    UFUNCTION(BlueprintCallable, Category = "Card UI")
    void RefreshCardDisplayData();

    // Data getters (for manual data access)
    UFUNCTION(BlueprintPure, Category = "Card UI Data")
    FCardDisplayData GetCardDisplayData() const { return CurrentDisplayData; }

    UFUNCTION(BlueprintPure, Category = "Card UI Data")
    bool IsPlayable() const { return CurrentDisplayData.bIsPlayable; }

    // Actions developers can call
    UFUNCTION(BlueprintCallable, Category = "Card UI Actions")
    void RequestCardInteraction();

    UFUNCTION(BlueprintCallable, Category = "Card UI Actions")
    void RequestCardHover();

    UFUNCTION(BlueprintCallable, Category = "Card UI Actions")
    void RequestCardUnhover();

    // Utility functions for getting display information
    UFUNCTION(BlueprintPure, Category = "Card UI Utils")
    FString GetCardTypeDisplayName(ECardType CardType) const;

    UFUNCTION(BlueprintPure, Category = "Card UI Utils")
    FString GetFactionDisplayName(ECardFaction Faction) const;

    UFUNCTION(BlueprintPure, Category = "Card UI Utils")
    FLinearColor GetFactionColor(ECardFaction Faction) const;

    UFUNCTION(BlueprintPure, Category = "Card UI Utils")
    bool ShouldShowAttackHealth(ECardType CardType) const;

protected:
    // Interface implementations (these call the data update system)
    void SetCardData_Implementation(const FCardData& CardData, int32 InHandIndex);
    void UpdateCardDisplay_Implementation();
    void SetPlayable_Implementation(bool bCanPlay);

private:
    // === HELPER FUNCTIONS ===

    FCardDisplayData BuildDisplayData() const;
    void BroadcastDisplayDataUpdate();
    bool HasDisplayDataChanged(const FCardDisplayData& NewData, const FCardDisplayData& OldData) const;

    FCardDisplayData LastDisplayData;
};