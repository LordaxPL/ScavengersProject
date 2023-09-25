// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Scavengers/Environment/Interactable.h"
#include "Openable.generated.h"

UCLASS()
class SCAVENGERS_API AOpenable : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AOpenable();

	virtual void Interact() override;
	bool IsLocked() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, category = "Mesh")
		UStaticMeshComponent* Mesh;

	UPROPERTY(EditInstanceOnly, category = "Interaction")
		bool bIsLocked;

	UPROPERTY(EditInstanceOnly, category = "Interaction")
		bool bFlipDirection;


	// Open the openable
	// bIsOpen, as oppoesd to bIsLocked, is used to determine whether the door is open or not
	// bIsLocked determines whether the player can open the door or has to lockpick it / usae a key on it
	bool bIsOpen;
	float LerpProgress;
	const float LerpIntensity = 0.0625f;
	// Ask your beloved brother which value is ok:
	// 0.03125f
	// 0.0625f
	// 0.125f

	FRotator OpenRotator;
	FRotator CloseRotator;
	FTimerHandle OpenCloseHandle;
	void LerpOpen();
	void LerpClose();
	void Open();
	void Close();





};