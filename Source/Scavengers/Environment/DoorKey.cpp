// Fill out your copyright notice in the Description page of Project Settings.


#include "Scavengers/Environment/DoorKey.h"
#include "Scavengers/Environment/Openable.h"

ADoorKey::ADoorKey()
{
	ItemID = 80;
	static ConstructorHelpers::FObjectFinder<UDataTable> DataTableObj(TEXT("DataTable'/Game/Assets/Pickables/Items.Items'"));
	if (DataTableObj.Succeeded())
	{
		ItemsData = DataTableObj.Object;
		FItemDataTableStruct* ItemsStruct = ItemsData->FindRow<FItemDataTableStruct>(FName(FString::FromInt(ItemID)), FString("Context"));
		Mesh->SetStaticMesh(ItemsStruct->Mesh);
	}
}
