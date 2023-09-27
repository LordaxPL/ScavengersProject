// Fill out your copyright notice in the Description page of Project Settings.


#include "Scavengers/Environment/MeleeWeapon.h"

AMeleeWeapon::AMeleeWeapon()
{
	ItemID = 81;
	static ConstructorHelpers::FObjectFinder<UDataTable> DataTableObj(TEXT("DataTable'/Game/Assets/Pickables/Items.Items'"));
	if (DataTableObj.Succeeded())
	{
		ItemsData = DataTableObj.Object;
		FItemDataTableStruct* ItemsStruct = ItemsData->FindRow<FItemDataTableStruct>(FName(FString::FromInt(ItemID)), FString("Context"));
		Mesh->SetStaticMesh(ItemsStruct->Mesh);
	}
}
bool AMeleeWeapon::Hit()
{
	return true;
}
bool AMeleeWeapon::HitHard()
{
	return true;
}

void AMeleeWeapon::Interact()
{
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "THIS IS A MELEE WEAPON");
}