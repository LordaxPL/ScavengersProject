// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Scavengers/Player/Inventory.h"
#include "Scavengers/Environment/Interactable.h"
#include "Scavengers/Environment/ItemEssentials.h"
#include "Pickable.generated.h"

//UENUM()
//enum ResourceType
//{
//	Food UMETA(DisplayName = "Food"),
//	Metal UMETA(DisplayName = "Metal"),
//	Wood UMETA(DisplayName = "Wood"),
//	ElectricalComponents UMETA(DisplayName = "ElectricalComponents"),
//	Special UMETA(DisplayName = "Special")
//};





UCLASS()
class SCAVENGERS_API APickable : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APickable();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* Mesh;

	UDataTable* ItemsData;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Interact() override;

	UPROPERTY(EditAnywhere)
	uint32 ItemID;

	UPROPERTY(EditInstanceOnly)
		int ID_Override = -1;

	void InitializeItem(int ID);



};
