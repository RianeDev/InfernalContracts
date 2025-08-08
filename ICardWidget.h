// ICardWidget.h - Interface for Card Widgets
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CardTypesHost.h"
#include "CardDisplayTypes.h"
#include "ICardWidget.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UCardWidget : public UInterface
{
    GENERATED_BODY()
};

class KEVESCARDKIT_API ICardWidget
{
    GENERATED_BODY()

public:
    // BlueprintNativeEvent allows C++ default implementation that Blueprint can override
    // C++ can call these, Blueprint can implement custom versions
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Card Widget")
    void SetCardData(const FCardDisplayData& CardDisplayData);
    virtual void SetCardData_Implementation(const FCardDisplayData& CardDisplayData) {}

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Card Widget")
    void UpdateCardDisplay();
    virtual void UpdateCardDisplay_Implementation() {}

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Card Widget")
    void SetPlayable(bool bCanPlay);
    virtual void SetPlayable_Implementation(bool bCanPlay) {}

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Card Widget")
    void SetHovered(bool bIsHovered);
    virtual void SetHovered_Implementation(bool bIsHovered) {}

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Card Widget")
    void SetSelected(bool bIsSelected);
    virtual void SetSelected_Implementation(bool bIsSelected) {}

    // Pure BlueprintImplementableEvent for data retrieval (Blueprint must implement)
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Card Widget")
    FCardDisplayData GetCurrentCardDisplayData() const;

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Card Widget")
    int32 GetCurrentHandIndex() const;
};