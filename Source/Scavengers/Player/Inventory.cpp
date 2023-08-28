// Fill out your copyright notice in the Description page of Project Settings.


#include "Scavengers/Player/Inventory.h"

// Sets default values for this component's properties
UInventory::UInventory()
{
	PrimaryComponentTick.bCanEverTick = false;
	MaxWeight = 100.0f;
	CurrentWeight = 0.0f;

	static ConstructorHelpers::FObjectFinder<UDataTable> ItemsTableObject(TEXT("DataTable'/Game/Assets/Pickables/Items.Items'"));
	if (ItemsTableObject.Succeeded())
	{
		ItemsTable = ItemsTableObject.Object;
	}
}

// Called when the game starts
void UInventory::BeginPlay()
{
	Super::BeginPlay();

}

bool UInventory::AddItem(uint32 ItemID, uint8 ItemAmount)
{
	// Getting information about the item from the item list
	FString ContextString("Item context");
	FName RowIndex = FName(FString::FromInt(ItemID));
	FItemDataTableStruct* ItemStruct = ItemsTable->FindRow<FItemDataTableStruct>(RowIndex, ContextString);

	// Checking if the item weight fits in the inventory
	if (CurrentWeight + ItemStruct->Size * ItemAmount > MaxWeight)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Item %d is too big to be placed in inventory!"), ItemID));
		return false;
	}
	else
	{
		// Checking if there's already an item of the right type in the inventory
		for (FItem Item : Items)
		{
			if (Item.ID == ItemID)
			{
				Item.Amount += ItemAmount;
				return true;
			}
		}

		// If no item was found, create a new one
		FItem ItemToAdd;
		ItemToAdd.ID = ItemID;
		ItemToAdd.Amount = ItemAmount;
		Items.Add(ItemToAdd);
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, "Add a function to update the inventory widget when adding an item");

		return true;
	}

}

void UInventory::RemoveItem(uint32 ItemID, bool bRemoveAll)
{
	int index = FindItemIndex(ItemID);
	if (index == -1)
	{
		return;
	}

	if (Items[index].Amount == 1 || bRemoveAll)
	{
		Items.RemoveAt(index);
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, "Add a function to update the inventory widget when removing an item");
	}
	else
	{
		Items[index].Amount -= 1;
	}
}

bool UInventory::CheckForItem(uint32 ItemID) const
{
	for (FItem Item : Items)
	{
		if (Item.ID == ItemID)
		{
			return true;
		}
	}
	return false;
}


float UInventory::GetCurrentWeight()
{
	return CurrentWeight;
}

float UInventory::GetMaxWeight()
{
	return MaxWeight;
}

int UInventory::FindItemIndex(uint32 ItemID)
{
	for (int i = 0; i < Items.Num(); i++)
	{
		if (Items[i].ID == ItemID)
		{
			return i;
		}
	}
	return -1;
}