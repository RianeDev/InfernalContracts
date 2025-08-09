#include "BattlefieldCardActor.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "CombatManager.h"

ABattlefieldCardActor::ABattlefieldCardActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize default values
    UniqueID = -1;
    BattlefieldIndex = -1;
    bIsPlayerOwned = true;
    OwningCombatManager = nullptr;

    // Ensure the capsule/sprite have proper collision in BP so clicks can hit this actor.
    // We'll rely on engine click detection (PlayerController->bEnableClickEvents = true).
}

void ABattlefieldCardActor::BeginPlay()
{
    Super::BeginPlay();

    // Optionally run initial visuals
    OnSpawnedVisuals();

    // If we have a CombatManager, bind to its UniqueID damage event
    if (ACombatManager* CombatMgr = Cast<ACombatManager>(OwningCombatManager))
    {
        CombatMgr->OnCardDamagedByUniqueID.AddDynamic(this, &ABattlefieldCardActor::OnCardDamagedByUniqueID);
    }
}

void ABattlefieldCardActor::InitializeBattlefieldCard(const FCardData& InCardData, int32 InUniqueID, int32 InIndex, bool bOwnedByPlayer, AActor* InCombatManager)
{
    CardData = InCardData;
    UniqueID = InUniqueID;
    BattlefieldIndex = InIndex;
    bIsPlayerOwned = bOwnedByPlayer;
    OwningCombatManager = InCombatManager;

    UE_LOG(LogTemp, Log, TEXT("[BattlefieldCardActor] Initialized: %s (UniqueID:%d, Index:%d, PlayerOwned:%s)"),
        *CardData.Name.ToString(), UniqueID, BattlefieldIndex, bOwnedByPlayer ? TEXT("Yes") : TEXT("No"));

    // Let Blueprints set flipbook/sprite/label based on CardData
    OnSpawnedVisuals();

    // Bind to combat manager's damage events
    if (ACombatManager* CombatMgr = Cast<ACombatManager>(OwningCombatManager))
    {
        if (!CombatMgr->OnCardDamagedByUniqueID.IsBound())
        {
            CombatMgr->OnCardDamagedByUniqueID.AddDynamic(this, &ABattlefieldCardActor::OnCardDamagedByUniqueID);
        }
    }
}

void ABattlefieldCardActor::OnSelectedByPlayer()
{
    UE_LOG(LogTemp, Log, TEXT("[BattlefieldCardActor] Selected UniqueID %d (Index:%d, PlayerSide:%s) for card %s"),
        UniqueID, BattlefieldIndex, bIsPlayerOwned ? TEXT("true") : TEXT("false"), *CardData.Name.ToString());

    // Broadcast delegate so CombatManager / PlayerController / UI can bind to it
    OnSelected.Broadcast(UniqueID, bIsPlayerOwned);
}

void ABattlefieldCardActor::UpdateBattlefieldIndex(int32 NewIndex)
{
    int32 OldIndex = BattlefieldIndex;
    BattlefieldIndex = NewIndex;

    UE_LOG(LogTemp, VeryVerbose, TEXT("[BattlefieldCardActor] %s (UniqueID:%d) index updated: %d -> %d"),
        *CardData.Name.ToString(), UniqueID, OldIndex, NewIndex);
}

void ABattlefieldCardActor::NotifyActorOnClicked(FKey ButtonPressed)
{
    Super::NotifyActorOnClicked(ButtonPressed);

    // When the actor is clicked, route to the selection handler.
    OnSelectedByPlayer();
}

void ABattlefieldCardActor::HandleDamaged(int32 DamageAmount)
{
    // Update card data
    int32 NewHealth = FMath::Max(0, CardData.Health - DamageAmount);
    CardData.Health = NewHealth;

    UE_LOG(LogTemp, Log, TEXT("[BattlefieldCardActor] %s (UniqueID:%d) took %d damage, health now %d"),
        *CardData.Name.ToString(), UniqueID, DamageAmount, NewHealth);

    // Fire delegate so Blueprint logic can run
    OnDamaged.Broadcast(NewHealth);
    OnHealthChanged(NewHealth, CardData.Health);

    // Optional: auto-play hit animation
    PlayHitAnimation();

    // If dead, optionally trigger death animation
    if (NewHealth <= 0)
    {
        PlayDeathAnimation();

        // Destroy after a short delay to allow animation to play
        FTimerHandle DestroyTimer;
        GetWorld()->GetTimerManager().SetTimer(DestroyTimer, [this]()
            {
                Destroy();
            }, 2.0f, false);
    }
}

void ABattlefieldCardActor::HandleHealed(int32 NewHealth)
{
    CardData.Health = NewHealth;

    UE_LOG(LogTemp, Log, TEXT("[BattlefieldCardActor] %s (UniqueID:%d) healed, health now %d"),
        *CardData.Name.ToString(), UniqueID, NewHealth);

    OnHealed.Broadcast(NewHealth);
    OnHealthChanged(NewHealth, CardData.Health);
}

// Event handler for when this card takes damage via UniqueID system
UFUNCTION()
void ABattlefieldCardActor::OnCardDamagedByUniqueID(int32 DamagedUniqueID, int32 DamageAmount)
{
    // Only respond if this is our card being damaged
    if (DamagedUniqueID == UniqueID)
    {
        HandleDamaged(DamageAmount);
    }
}