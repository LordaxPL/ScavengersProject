// Fill out your copyright notice in the Description page of Project Settings.


#include "Scavengers/Environment/ItemSpawner.h"
#include "Scavengers/Environment/Pickable.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AItemSpawner::AItemSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));

	static ConstructorHelpers::FObjectFinder<UDataTable> ItemsDataTableObject(TEXT("DataTable'/Game/Assets/Pickables/Items.Items'"));
	if (ItemsDataTableObject.Succeeded())
	{
		ItemsDataTable = ItemsDataTableObject.Object;
	}

}

// Called when the game starts or when spawned
void AItemSpawner::BeginPlay()
{
	Super::BeginPlay();

	int ItemCount = ItemsDataTable->GetRowNames().Num();
	int LocationsNum = ItemLocations.Num();
	int i = 0;

	// Initializing the values
	switch (ItemType)
	{
	case ResourceType::Food:
	default:
		MinRange = 0;
		MaxRange = 17;
		break;
	case ResourceType::Metal:
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "*** ADJUST MIN AND MAX RANGE OF METAL IN ItemSpawner!");
		MinRange = 20;
		MaxRange = 39;
		break;
	case ResourceType::Wood:
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "*** ADJUST MIN AND MAX RANGE OF WOOD IN ItemSpawner!");
		MinRange = 40;
		MaxRange = 59;
	case ResourceType::ElectricalComponents:
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "*** ADJUST MIN AND MAX RANGE OF ELECTRICAL IN ItemSpawner!");
		MinRange = 60;
		MaxRange = 79;
	case ResourceType::Special:
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "*** ADJUST MIN AND MAX RANGE OF SPECIAL IN ItemSpawner!");
		MinRange = 80;
		MaxRange = ItemCount;
	}

	// Adjusting the locations so they match the rotation of the parent object
	for (i = 0; i < LocationsNum; i++)
	{
		ItemLocations[i] = UKismetMathLibrary::Quat_RotateVector(FQuat(GetActorRotation()), ItemLocations[i]);
		ItemLocations[i] += GetActorLocation();
	}


	// Randomizing item count if it was set to -1
	if (ItemsCount == -1)
	{
		ItemsCount = FMath::RandRange(1, LocationsNum - 1);
	}
	
	if (ItemsCount > 0)
	{
		if (ItemsCount >= LocationsNum)
		{
			for (i = 0; i < LocationsNum; i++)
			{
				SpawnItem(&ItemLocations[i]);
			}
			ItemsCount = LocationsNum;
		}
		else
		{
			// If the number of items to spawn is greater than half of the number
			// of the locations, choose locations at which the item should not
			// spawn instead of locations to spawn the item at
			bool bLocationsBanned = false;
			int PlacesToChoose;
			if (LocationsNum - ItemsCount < LocationsNum / 2)
			{
				PlacesToChoose = LocationsNum - ItemsCount;
				bLocationsBanned = true;
			}
			else
			{
				PlacesToChoose = ItemsCount;
			}

			// Choosing ItemsCount locations

			int* ChosenLocations = new int[PlacesToChoose];

			for (i = 0; i < PlacesToChoose; i++)
			{
				ChosenLocations[i] = FMath::RandRange(1, LocationsNum - 1);
			}

			bool bRandomize;
			do
			{
				bRandomize = false;
				for (i = 0; i < PlacesToChoose; i++)
				{
					for (int c = 0; c < PlacesToChoose; c++)
					{
						if (ChosenLocations[i] == ChosenLocations[c] && i != c)
						{
							bRandomize = true;
							while (ChosenLocations[i] == ChosenLocations[c])
							{
								ChosenLocations[i] = FMath::RandRange(1, LocationsNum - 1);
							}
						}
					}
				}
			} while (bRandomize);

			// Spawning the items
			TArray<FVector*> FinalLocations;
			if (bLocationsBanned)
			{
				for (i = 0; i < ItemLocations.Num(); i++)
				{
					bool bCanAdd = true;
					for (int c = 0; c < PlacesToChoose; c++)
					{
						if (i == ChosenLocations[c])
						{
							bCanAdd = false;
						}
					}

					if (bCanAdd)
					{
						FinalLocations.Add(&ItemLocations[i]);
					}
				}
			}
			else
			{
				for (i = 0; i < PlacesToChoose; i++)
				{
					FinalLocations.Add(&ItemLocations[ChosenLocations[i]]);
				}
			}

			for (i = 0; i < FinalLocations.Num(); i++)
			{
				SpawnItem(FinalLocations[i]);
			}

			delete[] ChosenLocations;
		}
	}
}

int AItemSpawner::GetValidRandomID()
{
	bool bRepeat = false;
	int RandomID = FMath::RandRange(MinRange, MaxRange);
	for (int i = 0; i < BannedItems.Num(); i++)
	{
		if (RandomID == BannedItems[i])
		{
			bRepeat = true;
			break;
		}
	}

	if (bRepeat)
	{
		return GetValidRandomID();
	}
	else
	{
		return RandomID;
	}

}

void AItemSpawner::SpawnItem(FVector* SpawnLocation)
{
	FRotator NewRotation(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
	APickable* Item = GetWorld()->SpawnActor<APickable>(*SpawnLocation, NewRotation);
	if (Item != nullptr)
	{
		int ItemID = GetValidRandomID();
		Item->InitializeItem(ItemID);
	}
}


