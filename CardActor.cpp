// CardActor.cpp - Card Actor Implementation
#include "CardActor.h"
#include "KCKGameplayLibrary.h"
#include "HandManager.h"
#include "Kismet/GameplayStatics.h"

ACardActor::ACardActor()
{
    PrimaryActorTick.bCanEverTick = false;
    CardDataTable = nullptr;
    AbilityDataTable = nullptr;
}

void ACardActor::BeginPlay()
{
    Super::BeginPlay();
}

void ACardActor::InitializeCard(const FCardData& InCardData, UDataTable* InCardDataTable, UDataTable* InAbilityDataTable)
{
    CardData = InCardData;
    CardDataTable = InCardDataTable;
    AbilityDataTable = InAbilityDataTable;

    UE_LOG(LogTemp, Log, TEXT("[CardActor] Initialized card: %s (ID: %d, AbilityID: %d)"),
        *CardData.Name.ToString(), CardData.ID, CardData.AbilityID);
}

void ACardActor::ActivateAbility(AActor* Target)
{
    UE_LOG(LogTemp, Log, TEXT("[CardActor] ActivateAbility called for card: %s (AbilityID: %d)"),
        *CardData.Name.ToString(), CardData.AbilityID);
    
    if (CardData.AbilityID <= 0)
    {
        UE_LOG(LogTemp, Log, TEXT("[CardActor] Card '%s' has no ability (AbilityID: %d)"),
            *CardData.Name.ToString(), CardData.AbilityID);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[CardActor] Activating ability %d for card: %s"),
        CardData.AbilityID, *CardData.Name.ToString());

    // If no target is provided, try to find the HandManager in the world
    AActor* ActualTarget = Target;
    if (!ActualTarget)
    {
        if (UWorld* World = GetWorld())
        {
            TArray<AActor*> FoundActors;
            UGameplayStatics::GetAllActorsOfClass(World, AHandManager::StaticClass(), FoundActors);
            if (FoundActors.Num() > 0)
            {
                ActualTarget = FoundActors[0];
                UE_LOG(LogTemp, Log, TEXT("[CardActor] Found HandManager in world: %s"), *ActualTarget->GetName());
            }
        }
    }

    ExecuteAbilityByID(CardData.AbilityID, ActualTarget);
}

void ACardActor::ExecuteAbilityByID(int32 AbilityID, AActor* Target)
{
    if (!AbilityDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("[CardActor] No AbilityDataTable set!"));
        return;
    }

    FCardAbility* Ability = FindAbilityByID(AbilityID);
    if (!Ability)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CardActor] Ability ID %d not found in AbilityDataTable"), AbilityID);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[CardActor] Found ability: %s (Type: %d)"),
        *Ability->Name.ToString(), (int32)Ability->AbilityType);

    // Use the gameplay library to execute the ability
    UKCKGameplayLibrary::ExecuteAbilityByID(
        AbilityID,
        AbilityDataTable,
        CardDataTable,
        CardData,
        this, // Caster
        Target
    );

    UE_LOG(LogTemp, Log, TEXT("[CardActor] Ability execution completed for: %s"),
        *CardData.Name.ToString());
}

FCardAbility* ACardActor::FindAbilityByID(int32 AbilityID)
{
    if (!AbilityDataTable) return nullptr;

    for (const FName& RowName : AbilityDataTable->GetRowNames())
    {
        FCardAbility* FoundAbility = AbilityDataTable->FindRow<FCardAbility>(RowName, TEXT("FindAbilityByID"));
        if (FoundAbility && FoundAbility->ID == AbilityID)
        {
            return FoundAbility;
        }
    }
    return nullptr;
}