#include "KCKGameplayLibrary.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "HandManager.h"
#include "CardTypesHost.h"
#include "Engine/Level.h"
#include "Kismet/GameplayStatics.h"

void UKCKGameplayLibrary::ExecuteAbilityByID(int32 AbilityID, UDataTable* AbilityTable, UDataTable* CardTable, FCardData& CardData, AActor* Caster, AActor* Target)
{
    if (!AbilityTable || !Caster)
    {
        UE_LOG(LogTemp, Error, TEXT("[KCK] ExecuteAbilityByID: Missing AbilityTable or Caster"));
        return;
    }

    const FCardAbility* Ability = nullptr;

    // Find the ability by ID
    for (const FName& Row : AbilityTable->GetRowNames())
    {
        const FCardAbility* TestAbility = AbilityTable->FindRow<FCardAbility>(Row, TEXT("ExecuteAbility"));
        if (TestAbility && TestAbility->ID == AbilityID)
        {
            Ability = TestAbility;
            break;
        }
    }

    if (!Ability)
    {
        UE_LOG(LogTemp, Warning, TEXT("[KCK] Ability ID %d not found."), AbilityID);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[KCK] Executing ability: %s (ID: %d, Type: %d)"),
        *Ability->Name.ToString(), Ability->ID, (int32)Ability->AbilityType);
    UE_LOG(LogTemp, Log, TEXT("[KCK] - CardIDsToAffect: %d items"), Ability->CardIDsToAffect.Num());
    if (Ability->CardIDsToAffect.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("[KCK] - First CardID: %d"), Ability->CardIDsToAffect[0]);
    }
    UE_LOG(LogTemp, Log, TEXT("[KCK] - Count: %d"), Ability->Count);

    switch (Ability->AbilityType)
    {
    case ECardAbilityType::DrawSpecificCard:
        if (Ability->CardIDsToAffect.Num() > 0)
        {
            UE_LOG(LogTemp, Log, TEXT("[KCK] DrawSpecificCard: CardID %d, Count %d"),
                Ability->CardIDsToAffect[0], Ability->Count);
            DrawSpecificCard(CardTable, Ability->CardIDsToAffect[0], Ability->Count, Target);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[KCK] DrawSpecificCard: No CardIDsToAffect specified"));
        }
        break;

    case ECardAbilityType::DrawMultipleCards:
        UE_LOG(LogTemp, Log, TEXT("[KCK] DrawMultipleCards: %d card types, Count %d"),
            Ability->CardIDsToAffect.Num(), Ability->Count);
        DrawMultipleCards(CardTable, Ability->CardIDsToAffect, Ability->Count, Target);
        break;

    case ECardAbilityType::BuffAllCreatures:
        UE_LOG(LogTemp, Log, TEXT("[KCK] BuffAllCreatures: +%d ATK/HP"), Ability->Amount);
        BuffAllOwnedCreatures(Caster, Ability->Amount, Ability->Amount);
        break;

    case ECardAbilityType::ApplyDamage:
        UE_LOG(LogTemp, Log, TEXT("[KCK] ApplyDamage: %d damage"), Ability->Amount);
        ApplyDamage(Target, Ability->Amount);
        break;

    case ECardAbilityType::HealActor:
        UE_LOG(LogTemp, Log, TEXT("[KCK] HealActor: %d healing"), Ability->Amount);
        HealActor(Target, Ability->Amount);
        break;

    case ECardAbilityType::BuffCard:
        UE_LOG(LogTemp, Log, TEXT("[KCK] BuffCard: +%d ATK/HP"), Ability->Amount);
        BuffCard(CardData, Ability->Amount, Ability->Amount);
        break;

    case ECardAbilityType::None:
        UE_LOG(LogTemp, Log, TEXT("[KCK] Ability type is None - no effect"));
        break;

    default:
        UE_LOG(LogTemp, Warning, TEXT("[KCK] Unknown ability type: %d"), (int32)Ability->AbilityType);
        break;
    }
}

void UKCKGameplayLibrary::DrawSpecificCard(UDataTable* CardTable, int32 CardID, int32 Count, AActor* Target)
{
    UE_LOG(LogTemp, Log, TEXT("[KCK] DrawSpecificCard called - CardID: %d, Count: %d, Target: %s"),
        CardID, Count, Target ? *Target->GetName() : TEXT("nullptr"));
    
    if (!CardTable || Count <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[KCK] DrawSpecificCard: Invalid parameters"));
        return;
    }

    // Find the HandManager to add cards to
    AHandManager* HandManager = nullptr;
    if (Target && Target->IsA<AHandManager>())
    {
        HandManager = Cast<AHandManager>(Target);
        UE_LOG(LogTemp, Log, TEXT("[KCK] Found HandManager from Target parameter"));
    }
    else
    {
        // Try to find HandManager in the world
        if (UWorld* World = (Target ? Target->GetWorld() : nullptr))
        {
            TArray<AActor*> FoundActors;
            UGameplayStatics::GetAllActorsOfClass(World, AHandManager::StaticClass(), FoundActors);
            UE_LOG(LogTemp, Log, TEXT("[KCK] Found %d HandManager actors in world"), FoundActors.Num());
            if (FoundActors.Num() > 0)
            {
                HandManager = Cast<AHandManager>(FoundActors[0]);
                UE_LOG(LogTemp, Log, TEXT("[KCK] Using HandManager: %s"), *HandManager->GetName());
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[KCK] No World available to find HandManager"));
        }
    }

    // Find the card data
    for (const FName& Row : CardTable->GetRowNames())
    {
        const FCardData* Card = CardTable->FindRow<FCardData>(Row, TEXT("DrawSpecificCard"));

        if (Card && Card->ID == CardID)
        {
            for (int32 i = 0; i < Count; ++i)
            {
                if (HandManager)
                {
                    bool bAdded = HandManager->AddCardToHand(CardID);
                    UE_LOG(LogTemp, Log, TEXT("[KCK] Added card '%s' (ID %d) to hand: %s"),
                        *Card->Name.ToString(), Card->ID, bAdded ? TEXT("Success") : TEXT("Failed"));
                }
                else
                {
                    UE_LOG(LogTemp, Log, TEXT("[KCK] Drew card '%s' (ID %d) - No HandManager found"),
                        *Card->Name.ToString(), Card->ID);
                }
            }
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[KCK] Card ID %d not found in CardTable."), CardID);
}

void UKCKGameplayLibrary::DrawMultipleCards(UDataTable* CardTable, const TArray<int32>& CardIDs, int32 CountPerCard, AActor* Target)
{
    if (!CardTable || CardIDs.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[KCK] DrawMultipleCards: Invalid parameters"));
        return;
    }

    for (int32 CardID : CardIDs)
    {
        DrawSpecificCard(CardTable, CardID, CountPerCard, Target);
    }
}

void UKCKGameplayLibrary::BuffCard(FCardData& Card, int32 AttackIncrease, int32 HealthIncrease)
{
    Card.Attack += AttackIncrease;
    Card.Health += HealthIncrease;

    UE_LOG(LogTemp, Log, TEXT("[KCK] Buffed card %s to %d ATK / %d HP"),
        *Card.Name.ToString(), Card.Attack, Card.Health);
}

void UKCKGameplayLibrary::BuffAllOwnedCreatures(AActor* Owner, int32 AtkBoost, int32 HpBoost)
{
    if (!Owner) return;

    UE_LOG(LogTemp, Log, TEXT("[KCK] All owned creatures of %s gain +%d ATK / +%d HP"),
        *Owner->GetName(), AtkBoost, HpBoost);

    // TODO: Hook into actual card manager to loop through Owner's hand/field cards
    // This would need to be implemented based on your specific battlefield system
}

void UKCKGameplayLibrary::ApplyDamage(AActor* Target, int32 Amount)
{
    if (!Target || Amount <= 0) return;

    UE_LOG(LogTemp, Log, TEXT("[KCK] ApplyDamage: %s takes %d damage."), *Target->GetName(), Amount);

    // TODO: Implement actual damage application to Target
    // This might involve finding a health component or calling a damage function
}

void UKCKGameplayLibrary::HealActor(AActor* Target, int32 Amount)
{
    if (!Target || Amount <= 0) return;

    UE_LOG(LogTemp, Log, TEXT("[KCK] HealActor: %s heals %d HP."), *Target->GetName(), Amount);

    // TODO: Implement actual healing application to Target
    // This might involve finding a health component or calling a heal function
}