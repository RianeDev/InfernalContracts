// CardDisplayTypes.h - Shared UI Type Definitions
#pragma once

#include "CoreMinimal.h"
#include "CardTypesHost.h"
#include "CardDisplayTypes.generated.h"

// Data structure for card UI updates
USTRUCT(BlueprintType)
struct KEVESCARDKIT_API FCardDisplayData
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