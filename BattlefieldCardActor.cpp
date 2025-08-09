#include "BattlefieldCardActor.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

ABattlefieldCardActor::ABattlefieldCardActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Ensure the capsule/sprite have proper collision in BP so clicks can hit this actor.
    // We'll rely on engine click detection (PlayerController->bEnableClickEvents = true).
}

void ABattlefieldCardActor::BeginPlay()
{
    Super::BeginPlay();

    // Optionally run initial visuals
    OnSpawnedVisuals();
}

void ABattlefieldCardActor::InitializeBattlefieldCard(const FCardData& InCardData, int32 InIndex, bool bOwnedByPlayer, AActor* InCombatManager)
{
    CardData = InCardData;
    BattlefieldIndex = InIndex;
    bIsPlayerOwned = bOwnedByPlayer;
    OwningCombatManager = InCombatManager;

    // Let Blueprints set flipbook/sprite/label based on CardData
    OnSpawnedVisuals();
}

void ABattlefieldCardActor::OnSelectedByPlayer()
{
    UE_LOG(LogTemp, Log, TEXT("[BattlefieldActor] Selected index %d (playerSide: %s) for card %s"),
        BattlefieldIndex, bIsPlayerOwned ? TEXT("true") : TEXT("false"), *CardData.Name.ToString());

    // Broadcast delegate so CombatManager / PlayerController / UI can bind to it
    OnSelected.Broadcast(BattlefieldIndex, bIsPlayerOwned);

    // Optional: if owning manager exists and exposes a known function you want to call,
    // you can call it here (but we avoid hard dependency to keep this actor generic).
}

void ABattlefieldCardActor::SetBattlefieldIndex(int32 NewIndex)
{
    BattlefieldIndex = NewIndex;
}

void ABattlefieldCardActor::NotifyActorOnClicked(FKey ButtonPressed)
{
    Super::NotifyActorOnClicked(ButtonPressed);

    // When the actor is clicked, route to the selection handler.
    OnSelectedByPlayer();
}

void ABattlefieldCardActor::HandleDamaged(int32 DamageAmount)
{
    // Fire delegate so Blueprint logic can run
    int32 NewHealth = FMath::Max(0, CardData.Health - DamageAmount);
    CardData.Health = NewHealth;

    OnDamaged.Broadcast(NewHealth);

    // Optional: auto-play hit animation
    PlayHitAnimation();

    // If dead, optionally trigger death animation
    if (NewHealth <= 0)
    {
        PlayDeathAnimation();
    }
}

void ABattlefieldCardActor::HandleHealed(int32 NewHealth)
{
    OnHealed.Broadcast(NewHealth);
}
