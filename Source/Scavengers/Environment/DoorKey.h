// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Scavengers/Environment/Pickable.h"
#include "DoorKey.generated.h"

/**
 * 
 */
UCLASS()
class SCAVENGERS_API ADoorKey : public APickable
{
	GENERATED_BODY()
public:
		ADoorKey();

		UPROPERTY(EditAnywhere)
		class AOpenable* DoorToOpen;
};
