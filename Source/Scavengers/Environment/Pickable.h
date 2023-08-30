// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Scavengers/Player/Inventory.h"
#include "Pickable.generated.h"

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
		float Weight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		class UTexture2D* Image;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UStaticMesh* Mesh;

};

UCLASS()
class SCAVENGERS_API APickable : public AActor
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

	void Interact();

	uint32 ItemID;

	UPROPERTY(EditInstanceOnly)
		int ID_Override = -1;

	void InitializeItem(uint32 ID);



};
