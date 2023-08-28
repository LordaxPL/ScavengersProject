// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Inventory.generated.h"

UENUM()
enum ResourceType
{
	Food UMETA(DisplayName = "Food"),
	Metal UMETA(DisplayName = "Metal"),
	Wood UMETA(DisplayName = "Wood"),
	ElectricalComponents UMETA(DisplayName = "ElectricalComponents"),
	Special UMETA(DisplayName = "Special")
};

USTRUCT(BlueprintType)
struct FItemDataTableStruct : public FTableRowBase
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TEnumAsByte<ResourceType> Type;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		uint8 Size;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		class UTexture2D* Image;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UStaticMesh* Mesh;

};

struct FItem
{
	uint32 ID;
	uint8 Amount;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SCAVENGERS_API UInventory : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInventory();

	// Functions to manage inventory
	// Returns true if added the item without issues, false if there was no space in the inventory
	bool AddItem(uint32 ItemID, uint8 ItemAmount = 1);
	void RemoveItem(uint32 ItemID, bool bRemoveAll = false);

	// Returns true if the item was found in the inventory
	bool CheckForItem(uint32 ItemID) const;
	float GetCurrentWeight();
	float GetMaxWeight();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// holds all the items with their IDs
	TArray<FItem> Items;

	UDataTable* ItemsTable;

	void UpdateWeight();

	// Finds position of the item in the array by its ID, returns -1 if the item was not found
	int FindItemIndex(uint32 ItemID);

	// variables
	float MaxWeight;
	float CurrentWeight;

		
};
