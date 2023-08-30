// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Inventory.generated.h"



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

	// Returns true if the item was found in the inventory
	bool CheckForItem(uint32 ItemID) const;
	float GetCurrentWeight();
	float GetMaxWeight();

	TArray<FItem>* GetAllItems();

	// Inventory
	// Shows or hides inventory tab
	// Returns true if the inventory was displayed or false when it was hidden
	bool ToggleInventory();

	// Checks if CurrentWeight > MaxWeight
	bool IsOverencumbered();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// holds all the items with their IDs
	TArray<FItem> Items;

	UDataTable* ItemsTable;

	// Finds position of the item in the array by its ID, returns -1 if the item was not found
	int FindItemIndex(uint32 ItemID);

	// variables
	float MaxWeight;
	float CurrentWeight;

	// Inventory
	class UUserWidget* InventoryWidget;
	UPROPERTY(EditAnywhere, category = "UI")
		TSubclassOf<UUserWidget> InventoryWidgetClass;

	class UUserWidget* InventorySlot;
	UPROPERTY(EditAnywhere, category = "UI")
		TSubclassOf<UUserWidget> InventorySlotClass;

	class UScrollBox* InventoryScrollBox;
	class UButton* DropButton;

	// Adds InventorySlot widgets to InventoryTab widget
	void PopulateInventory();

	// Clears all InventorySlots in InventoryTab widget
	void DePopulateInventory();

	UFUNCTION()
	void OnSlotPressed();

	UFUNCTION()
	void OnDropButtonPressed();

	int SelectedSlotID;

	// Removes an item at given index
	void DropItem(int32 index);
	// Spawns an item next to the player
	void SpawnItem(int32 itemID);
	// Refreshes the widget and updates all slots. Always call it after making a change to the inventory (adding / removing items)
	void RefreshInventoryWidget();

		
};
