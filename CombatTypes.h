// CombatTypes.h - Combat-related enums and types
#pragma once

#include "CoreMinimal.h"
#include "CombatTypes.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    None            UMETA(DisplayName = "None"),
    Starting        UMETA(DisplayName = "Starting"),
    PlayerTurn      UMETA(DisplayName = "Player Turn"),
    EnemyTurn       UMETA(DisplayName = "Enemy Turn"),
    Victory         UMETA(DisplayName = "Victory"),
    Defeat          UMETA(DisplayName = "Defeat")
}; 