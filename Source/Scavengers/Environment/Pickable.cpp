// Fill out your copyright notice in the Description page of Project Settings.


#include "Scavengers\Environment\Pickable.h"

// Sets default values
APickable::APickable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	ItemID = 0;

	static ConstructorHelpers::FObjectFinder<UDataTable> DataTableObj(TEXT("DataTable'/Game/Assets/Pickables/Items.Items'"));
	if (DataTableObj.Succeeded())
	{
		ItemsData = DataTableObj.Object;
	}

}

// Called when the game starts or when spawned
void APickable::BeginPlay()
{
	Super::BeginPlay();

	if (ID_Override != -1)
	{
		ItemID = ID_Override;
		InitializeItem(ItemID);
	}


	
}

// Called every frame
void APickable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Tick has been turned off in the constructor

}

void APickable::Interact()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, "Make APickable derive Interact() from UInteractable");
	Destroy();
}

void APickable::InitializeItem(uint32 ID)
{
	if (ItemsData != nullptr)
	{
		FItemDataTableStruct* ItemStruct;
		FString ContextStr("Context");
		FName RowName = FName(FString::FromInt(ID));
		ItemStruct = ItemsData->FindRow<FItemDataTableStruct>(RowName, ContextStr);
		Mesh->SetStaticMesh(ItemStruct->Mesh);
	}
}

