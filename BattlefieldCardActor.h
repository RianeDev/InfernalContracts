#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "CardDisplayTypes.h"   // contains FCardDisplayData and includes CardTypesHost which defines FCardData
#include "BattlefieldCardActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBattlefieldCardSelected, int32, BattlefieldIndex, bool, bIsPlayerOwned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattlefieldCardDamaged, int32, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattlefieldCardHealed, int32, NewHealth);

/**
 * Visual-only battlefield card actor (PaperCharacter).
 * Holds identifying info (index/side/card data) but NOT authoritative gameplay state.
 * CombatManager keeps gameplay state; this actor is for visuals & selection.
 */
UCLASS()
class KEVESCARDKIT_API ABattlefieldCardActor : public APaperCharacter
{
    GENERATED_BODY()

public:
    ABattlefieldCardActor();

    /** Broadcast when this card takes damage */
    UPROPERTY(BlueprintAssignable, Category = "Battlefield|Events")
    FOnBattlefieldCardDamaged OnDamaged;

    /** Broadcast when this card is healed */
    UPROPERTY(BlueprintAssignable, Category = "Battlefield|Events")
    FOnBattlefieldCardHealed OnHealed;

    /** Index in CombatManager's battlefield array (keeps in sync with gameplay array) */
    UPROPERTY(BlueprintReadOnly, Category = "Battlefield")
    int32 BattlefieldIndex = -1;

    /** True if this actor belongs to player side */
    UPROPERTY(BlueprintReadOnly, Category = "Battlefield")
    bool bIsPlayerOwned = false;

    /** Card data used for visuals (non-authoritative) */
    UPROPERTY(BlueprintReadOnly, Category = "Battlefield")
    FCardData CardData;

    /** Optional pointer to the CombatManager that spawned this actor (not required) */
    UPROPERTY(BlueprintReadOnly, Category = "Battlefield")
    AActor* OwningCombatManager = nullptr;

    /** Delegate broadcast when the actor is selected (clicked) */
    UPROPERTY(BlueprintAssignable, Category = "Battlefield")
    FOnBattlefieldCardSelected OnSelected;

    /** Initialize actor after spawn (set index/side/card, optional manager pointer) */
    UFUNCTION(BlueprintCallable, Category = "Battlefield")
    void InitializeBattlefieldCard(const FCardData& InCardData, int32 InIndex, bool bOwnedByPlayer, AActor* InCombatManager = nullptr);

    /** Called by PC or by engine when actor is clicked (exposed for BP) */
    UFUNCTION(BlueprintCallable, Category = "Battlefield")
    void OnSelectedByPlayer();

    /** Update index if manager removes/compacts array (keeps actor index in sync) */
    UFUNCTION(BlueprintCallable, Category = "Battlefield")
    void SetBattlefieldIndex(int32 NewIndex);

    /** Called by CombatManager when this card’s health changes due to damage */
    UFUNCTION(BlueprintCallable, Category = "Battlefield|Gameplay")
    void HandleDamaged(int32 DamageAmount);

    /** Called by CombatManager when this card’s health changes due to healing */
    UFUNCTION(BlueprintCallable, Category = "Battlefield|Gameplay")
    void HandleHealed(int32 NewHealth);

    /** Play hit/death animations in BP */
    UFUNCTION(BlueprintImplementableEvent, Category = "Battlefield|VFX")
    void PlayHitAnimation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Battlefield|VFX")
    void PlayDeathAnimation();

    /** Optional event BPs can use to set sprite/flipbook/text on spawn */
    UFUNCTION(BlueprintImplementableEvent, Category = "Battlefield|VFX")
    void OnSpawnedVisuals();

protected:
    virtual void BeginPlay() override;

    /** Route clicks (player controller must have "Enable Click Events" set) */
    virtual void NotifyActorOnClicked(FKey ButtonPressed) override;
};

