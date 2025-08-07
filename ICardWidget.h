// ICardWidget.h - Interface for Card Widgets
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CardTypesHost.h"
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
    // Called when the widget should display new card data
    UFUNCTION(BlueprintImplementableEvent, Category = "Card Widget")
    void SetCardData(const FCardData& CardData, int32 HandIndex);

    // Called when the widget should update its visual state
    UFUNCTION(BlueprintImplementableEvent, Category = "Card Widget")
    void UpdateCardDisplay();

    // Called when the card becomes playable/unplayable
    UFUNCTION(BlueprintImplementableEvent, Category = "Card Widget")
    void SetPlayable(bool bCanPlay);
};