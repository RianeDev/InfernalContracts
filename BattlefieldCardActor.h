// BattlefieldCardActor.h - Updated for Unique ID system
#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "CardTypesHost.h"
#include "BattlefieldCardActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCardSelected, int32, UniqueID, bool, bIsPlayerOwned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardDamagedActor, int32, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardHealedActor, int32, NewHealth);

UCLASS(BlueprintType, Blueprintable)
class KEVESCARDKIT_API ABattlefieldCardActor : public APaperCharacter
{
    GENERATED_BODY()

public:
    ABattlefieldCardActor();

protected:
    virtual void BeginPlay() override;

public:
    // === CARD DATA ===
    UPROPERTY(BlueprintReadOnly, Category = "Card Data")
    FCardData CardData;

    // NEW: Use Unique ID instead of array index
    UPROPERTY(BlueprintReadOnly, Category = "Card Data")
    int32 UniqueID = -1;

    // Array index (for reference, but not used for targeting)
    UPROPERTY(BlueprintReadOnly, Category = "Card Data")
    int32 BattlefieldIndex = -1;

    UPROPERTY(BlueprintReadOnly, Category = "Card Data")
    bool bIsPlayerOwned = true;

    UPROPERTY(BlueprintReadWrite, Category = "Card Data")
    AActor* OwningCombatManager = nullptr;

    // === EVENTS ===
    UPROPERTY(BlueprintAssignable, Category = "Card Events")
    FOnCardSelected OnSelected;

    UPROPERTY(BlueprintAssignable, Category = "Card Events")
    FOnCardDamagedActor OnDamaged;

    UPROPERTY(BlueprintAssignable, Category = "Card Events")
    FOnCardHealedActor OnHealed;

    // === INITIALIZATION ===
    UFUNCTION(BlueprintCallable, Category = "Battlefield Card")
    void InitializeBattlefieldCard(const FCardData& InCardData, int32 InUniqueID, int32 InIndex, bool bOwnedByPlayer, AActor* InCombatManager = nullptr);

    // === INTERACTION ===
    UFUNCTION(BlueprintCallable, Category = "Battlefield Card")
    void OnSelectedByPlayer();

    // === DAMAGE HANDLING ===
    UFUNCTION(BlueprintCallable, Category = "Battlefield Card")
    void HandleDamaged(int32 DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Battlefield Card")
    void HandleHealed(int32 NewHealth);

    // === UTILITY ===
    UFUNCTION(BlueprintPure, Category = "Battlefield Card")
    int32 GetUniqueID() const { return UniqueID; }

    UFUNCTION(BlueprintCallable, Category = "Battlefield Card")
    void UpdateBattlefieldIndex(int32 NewIndex);

    // === BLUEPRINT EVENTS ===
    UFUNCTION(BlueprintImplementableEvent, Category = "Battlefield Card")
    void OnSpawnedVisuals();

    UFUNCTION(BlueprintImplementableEvent, Category = "Battlefield Card")
    void PlayHitAnimation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Battlefield Card")
    void PlayDeathAnimation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Battlefield Card")
    void OnHealthChanged(int32 NewHealth, int32 MaxHealth);

protected:
    virtual void NotifyActorOnClicked(FKey ButtonPressed = EKeys::LeftMouseButton) override;

    // Event handler for damage by UniqueID
    UFUNCTION()
    void OnCardDamagedByUniqueID(int32 DamagedUniqueID, int32 DamageAmount);
};