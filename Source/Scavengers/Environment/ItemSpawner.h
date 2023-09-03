// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Scavengers/Environment/ItemEssentials.h"
#include "ItemSpawner.generated.h"

UCLASS()
class SCAVENGERS_API AItemSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category="Spawning", meta=(MakeEditWidget))
		TArray<FVector> ItemLocations;

	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
		TArray<int> BannedItems;

	UPROPERTY(EditAnywhere, Category="Mesh")
		UStaticMeshComponent* StaticMesh;



	class UDataTable* ItemsDataTable;

	// Max and min range of items to spawn
	int MaxRange;
	int MinRange;

	// Determines how many items will be spawned (random if 0)
	UPROPERTY(EditInstanceOnly, Category="Spawning")
	int ItemsCount;

	UPROPERTY(EditDefaultsOnly, Category="Spawning")
	TEnumAsByte<ResourceType> ItemType;

	int GetValidRandomID();
	void PopulateWithItems(int LocationIndexesNum, int* InLocationIndexes, bool bAreLocationsBanned = false);
	void SpawnItem(FVector* SpawnLocation);

};
