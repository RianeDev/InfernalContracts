// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CardTypesHost.h"
#include "CardActor.h"
#include "HandManager.generated.h"

UCLASS()
class KEVESCARDKIT_API AHandManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHandManager();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card System")
	TSubclassOf<ACardActor> CardActorClass;

	UPROPERTY()
	TArray<FCardData> CurrentHand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card System")
	UDataTable* CardDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card System")
	UDataTable* AbilityDataTable;

	UFUNCTION(BlueprintCallable, Category = "Card System")
	void AddCardByID(int32 CardID, int32 IndexInHand = 0);

	UFUNCTION(BlueprintCallable, Category = "Card System")
	void RemoveCardByID(int32 CardID, int32 IndexInHand = 0);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
};
