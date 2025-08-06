// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CardTypesHost.generated.h"

UENUM(BlueprintType)
enum class ECardType : uint8
{
    Creature     UMETA(DisplayName = "Creature"),
    Magic        UMETA(DisplayName = "Magic"),
    Power        UMETA(DisplayName = "Power"),
    Skill        UMETA(DisplayName = "Skill"),
    Status       UMETA(DisplayName = "Status"),
    Curse        UMETA(DisplayName = "Curse")
};

UENUM(BlueprintType)
enum class ECardRarity : uint8
{
    Common       UMETA(DisplayName = "Common"),
    Rare         UMETA(DisplayName = "Rare"),
    Epic         UMETA(DisplayName = "Epic"),
    Legendary    UMETA(DisplayName = "Legendary")
};

UENUM(BlueprintType)
enum class ECardFaction : uint8
{
    Demon        UMETA(DisplayName = "Demon"),
    Undead       UMETA(DisplayName = "Undead"),
    Angel        UMETA(DisplayName = "Angel")
};

UENUM(BlueprintType)
enum class ECardAbilityType : uint8
{
    None                UMETA(DisplayName = "None"),
    DrawSpecificCard    UMETA(DisplayName = "Draw Specific Card"),
    DrawMultipleCards   UMETA(DisplayName = "Draw Multiple Cards"),
    BuffAllCreatures    UMETA(DisplayName = "Buff All Owned Creatures"),
    ApplyDamage         UMETA(DisplayName = "Apply Damage"),
    HealActor           UMETA(DisplayName = "Heal Actor"),
    BuffCard            UMETA(DisplayName = "Buff Single Card"),

    // Add more as needed...
};


USTRUCT(BlueprintType)
struct FCardData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 ID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    ECardType CardType = ECardType::Creature;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    ECardRarity CardRarity = ECardRarity::Common;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    ECardFaction CardFaction = ECardFaction::Demon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 AbilityID = -1; // -1 = no ability

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Attack = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Health = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Cost = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UTexture2D* CardArt = nullptr;
};

USTRUCT(BlueprintType)
struct FCardAbility : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECardAbilityType AbilityType = ECardAbilityType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> CardIDsToAffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Amount = 0;
};