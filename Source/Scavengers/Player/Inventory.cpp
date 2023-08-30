// Fill out your copyright notice in the Description page of Project Settings.


#include "Scavengers/Player/Inventory.h"
#include "Blueprint/UserWidget.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Scavengers\Environment\Pickable.h"
#include "DrawDebugHelpers.h"



// Sets default values for this component's properties
UInventory::UInventory()
{
	PrimaryComponentTick.bCanEverTick = false;
	MaxWeight = 30U;
	CurrentWeight = 0U;
	SelectedSlotID = -1;

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

	// Initializing the inventory widget
	if (IsValid(InventoryWidgetClass))
	{
		InventoryWidget = CreateWidget(GetWorld(), InventoryWidgetClass);

		if (IsValid(InventoryWidget))
		{
			InventoryScrollBox = Cast<UScrollBox>(InventoryWidget->GetWidgetFromName(FName("InventoryScrollBox")));
			DropButton = Cast<UButton>(InventoryWidget->GetWidgetFromName(FName("DropButton")));

			if (DropButton != nullptr)
			{
				DropButton->OnPressed.AddDynamic(this, &UInventory::OnDropButtonPressed);
			}
		}
	}



}

bool UInventory::AddItem(uint32 ItemID, uint8 ItemAmount)
{
	// Getting information about the item from the item list
	FString ContextString("Item context");
	FName RowIndex = FName(FString::FromInt(ItemID));
	FItemDataTableStruct* ItemStruct = ItemsTable->FindRow<FItemDataTableStruct>(RowIndex, ContextString);

	// Checking if the ID is valid
	if (ItemStruct == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Item %d - wrong ID!"), ItemID));
		return false;
	}
	else
	{
		// Adding weight of the item to current weight
		CurrentWeight += ItemStruct->Weight;

		// Checking if there's already an item of the right type in the inventory
		for (int i = 0; i < Items.Num(); i++)
		{
			if (Items[i].ID == ItemID)
			{
				Items[i].Amount += ItemAmount;
				return true;
			}
		}

		// If no item was found, create a new one
		FItem ItemToAdd;
		ItemToAdd.ID = ItemID;
		ItemToAdd.Amount = ItemAmount;
		Items.Add(ItemToAdd);

		return true;
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

TArray<FItem>* UInventory::GetAllItems()
{
	return &Items;
}

bool UInventory::ToggleInventory()
{
	if (IsValid(InventoryWidget))
	{
		if (InventoryWidget->IsInViewport())
		{
			InventoryWidget->RemoveFromViewport();
			DePopulateInventory();
			return false;
		}
		else
		{
			InventoryWidget->AddToViewport(1);
			UTextBlock* Text = Cast<UTextBlock>(InventoryWidget->GetWidgetFromName("InventoryWeight"));
			if (Text)
			{
				Text->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), int(CurrentWeight), int(MaxWeight))));
				if (CurrentWeight > MaxWeight)
				{
					Text->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
				}
				else if (CurrentWeight == MaxWeight)
				{
					Text->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
				}
			}
			PopulateInventory();
			return true;
		}
	}
	return false;
}

void UInventory::PopulateInventory()
{
	FItemDataTableStruct* ItemStruct;
	for (int i = 0; i < Items.Num(); i++)
	{
		FName Row = FName(FString::FromInt(Items[i].ID));
		FString Context("Context");
		ItemStruct = ItemsTable->FindRow<FItemDataTableStruct>(Row, Context);
		
		InventorySlot = CreateWidget(GetWorld(), InventorySlotClass);
		UButton* SlotButton = Cast<UButton>(InventorySlot->GetRootWidget());
		if (SlotButton)
		{
			SlotButton->OnPressed.AddDynamic(this, &UInventory::OnSlotPressed);
		}

		UImage* Icon = Cast<UImage>(InventorySlot->GetWidgetFromName("Icon"));
		Icon->SetBrushFromTexture(ItemStruct->Image);

		UTextBlock* Text = Cast<UTextBlock>(InventorySlot->GetWidgetFromName("Name")); 
		Text->SetText(FText::FromString(ItemStruct->Name));

		Text = Cast<UTextBlock>(InventorySlot->GetWidgetFromName("Type"));
		FString TextToSet;
		switch (ItemStruct->Type)
		{
		case ResourceType::Food:
			TextToSet = "Food";
			break;

		case ResourceType::Wood:
			TextToSet = "Wood";
			break;

		case ResourceType::Metal:
			TextToSet = "Metal";
			break;

		case ResourceType::ElectricalComponents:
			TextToSet = "Electronics";
			break;

		case ResourceType::Special:
			TextToSet = "Special";
			break;

		default:
			TextToSet = "UNKNOWN";
			break;
		}
		Text->SetText(FText::FromString(TextToSet));

		Text = Cast<UTextBlock>(InventorySlot->GetWidgetFromName("Amount"));
		Text->SetText(FText::FromString(FString::FromInt(Items[i].Amount)));

		Text = Cast<UTextBlock>(InventorySlot->GetWidgetFromName("Weight"));
		float Weight = Items[i].Amount * ItemStruct->Weight;
		Text->SetText(FText::FromString(FString::SanitizeFloat(Weight)));
		
		InventoryScrollBox->AddChild(InventorySlot);
	}
}

void UInventory::DePopulateInventory()
{
	InventoryScrollBox->ClearChildren();

	// If to do it in a for loop, remember to always delete the child at index 0
	// Or delete them from the highest index to the lowest, as after deleting
	// a child, all indexes get adjusted, for example index 2 -> index 1
}

bool UInventory::IsOverencumbered()
{
	if (CurrentWeight > MaxWeight)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void UInventory::OnSlotPressed()
{
	UButton* Button;
	for (int i = 0; i < InventoryScrollBox->GetChildrenCount(); i++)
	{
		UUserWidget* Slot = Cast<UUserWidget>(InventoryScrollBox->GetChildAt(i));
		Button = Cast<UButton>(Slot->GetRootWidget());

		if (Button && Button->IsPressed())
		{
			SelectedSlotID = i;
		}
	}
}

void UInventory::OnDropButtonPressed()
{
	if (SelectedSlotID != -1)
	{
		DropItem(SelectedSlotID);
	}
}

void UInventory::DropItem(int32 index)
{
	if (index == Items.Num() && Items.Num() != 0)
	{
		SelectedSlotID--;
		DropItem(SelectedSlotID);
	}
	else if (index < Items.Num())
	{
		SpawnItem(Items[index].ID);
		if (Items[index].Amount == 1)
		{
			Items.RemoveAt(index);
			if (InventoryScrollBox->GetChildAt(index) != nullptr)
			{
				SelectedSlotID = index;
			}
			else if (InventoryScrollBox->GetChildAt(index - 1) != nullptr)
			{
				SelectedSlotID = index - 1;
			}
			else
			{
				SelectedSlotID = -1;
			}
		}
		else
		{
			Items[index].Amount -= 1;
		}
		RefreshInventoryWidget();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, "WRONG INDEX, ITEM WAS NOT DROPPED");
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("INDEX: %d"), index));
	}
}

void UInventory::RefreshInventoryWidget()
{
	InventoryScrollBox->ClearChildren();
	PopulateInventory();
}

void UInventory::SpawnItem(int32 itemID)
{
	FHitResult SpawnCheckResult;
	FCollisionQueryParams SpawnCheckParams;
	AActor* Owner = GetOwner();
	SpawnCheckParams.AddIgnoredActor(Owner);
	FVector PlayerLocation = Owner->GetActorLocation();
	FRotator PlayerRotation = Owner->GetActorRotation();
	FVector SpawnCheckStart = PlayerLocation;
	SpawnCheckStart.Z += 100.0f;
	FVector SpawnCheckEnd = SpawnCheckStart;
	SpawnCheckEnd.Z -= 2000.0f;
	DrawDebugLine(GetWorld(), SpawnCheckStart, SpawnCheckEnd, FColor::Red, true);
	DrawDebugLine(GetWorld(), PlayerLocation, SpawnCheckStart, FColor::Green, true);
	do
	{
		float RandX = FMath::RandRange(32.0f, 48.0f);
		float RandY = FMath::RandRange(32.0f, 48.0f);
		if (FMath::RandBool())
		{
			RandX *= -1;
		}
		if (FMath::RandBool())
		{
			RandY *= -1;
		}
		SpawnCheckStart.X = PlayerLocation.X + RandX;
		SpawnCheckStart.Y = PlayerLocation.Y + RandY;
		SpawnCheckEnd.X = SpawnCheckStart.X;
		SpawnCheckEnd.Y = SpawnCheckStart.Y;
		DrawDebugLine(GetWorld(), SpawnCheckStart, SpawnCheckEnd, FColor::Red, true);
	} while (!GetWorld()->LineTraceSingleByChannel(SpawnCheckResult, SpawnCheckStart,
		SpawnCheckEnd, ECollisionChannel::ECC_Visibility, SpawnCheckParams));

	FActorSpawnParameters SpawnParams;
	APickable* SpawnedPickable = Cast<APickable>(GetWorld()->SpawnActor<APickable>(
		SpawnCheckResult.ImpactPoint, PlayerRotation, SpawnParams));

	DrawDebugBox(GetWorld(), SpawnCheckResult.ImpactPoint, FVector(5.0f), FColor::Purple, true);

	if (SpawnedPickable)
	{
		SpawnedPickable->ItemID = itemID;
		SpawnedPickable->InitializeItem(itemID);
	}
	

}