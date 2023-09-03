// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemEssentials.generated.h"

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

/**
 * 
 */
class SCAVENGERS_API ItemEssentials
{
public:
	ItemEssentials();
	~ItemEssentials();
};
