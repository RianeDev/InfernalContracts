// CardUIWidget.h - Updated to make CurrentDisplayData BlueprintReadWrite
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ICardWidget.h"
#include "CardTypesHost.h"
#include "CardDisplayTypes.h"
#include "CardUIWidget.generated.h"

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
    // === CARD DATA === - Changed to BlueprintReadWrite as requested

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

    // New function to force playable state (useful for debugging)
    UFUNCTION(BlueprintCallable, Category = "Card UI Debug")
    void ForcePlayableState(bool bForcePlayable);

protected:
    // Interface implementations - these provide default C++ behavior
    virtual void SetCardData_Implementation(const FCardDisplayData& CardDisplayData) override;
    virtual void UpdateCardDisplay_Implementation() override;
    virtual void SetPlayable_Implementation(bool bCanPlay) override;
    virtual void SetHovered_Implementation(bool bIsHovered) override;
    virtual void SetSelected_Implementation(bool bIsSelected) override;

private:
    // === HELPER FUNCTIONS ===

    FCardDisplayData BuildDisplayData() const;
    void BroadcastDisplayDataUpdate();
    bool HasDisplayDataChanged(const FCardDisplayData& NewData, const FCardDisplayData& OldData) const;

    FCardDisplayData LastDisplayData;
};