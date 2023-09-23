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
	bool AddKey(AActor* DoorToOpen);
	// returns id of the slot the weapon was added to. Returns 4, if the weapon was added to the inventory
	uint8 AddWeapon(uint8 WeaponID);
	// Finds a key to a certain door, if bRemoveKey is set to true, removes the key from the inventory
	bool FindDoorKey(AActor* Door, bool bRemoveKey = true);

	// Checks if the player has the weapon (if the slot of this weapon is claimed)
	// Slots are ranged from 0 to 2
	bool IsWeaponSlotFree(uint8 Slot = 0);
	void SwitchWeapon(bool bUp = true);



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

	// Refreshes the widget and updates all slots. Always call it after making a change to the inventory (adding / removing items)
	void RefreshInventoryWidget();
	bool IsVisible();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// holds all the items with their IDs
	TArray<FItem> Items;
	// Stores pointers to doors opened by keys
	TArray<AActor*> DoorKeys;

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
	class UTextBlock* WeightText;

	// Adds InventorySlot widgets to InventoryTab widget
	void PopulateInventory();

	// Clears all InventorySlots in InventoryTab widget
	void DePopulateInventory();

	// Adds an inventory slot used for displaying items of one type
	void CreateInventorySlot(UTexture2D* Image, FString& Name, FString& Type, int Amount, float Weight);

	UFUNCTION()
	void OnSlotPressed();

	UFUNCTION()
	void OnDropButtonPressed();

	int SelectedSlotID;

	// Removes an item at given index
	void DropItem(int32 index);
	// Spawns an item next to the player
	void SpawnItem(int32 itemID);
	void UpdateInventoryWeight();
	
	struct FItemDataTableStruct* FindItemInTable(int ID);

	// Weapons and tools
	uint8 WeaponSlots[3];


		
};
