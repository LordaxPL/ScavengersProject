// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "C:\Users\ACER\Documents\Unreal Projects\Scavengers\Source\Scavengers\ArtificialIntelligence\LivingBeing.h"
#include "PlayerCharacter.generated.h"

enum StatStatus
{
	Stable,
	Regenerating,
	Draining,
	Depleted
};

UCLASS()
class SCAVENGERS_API APlayerCharacter : public ACharacter, public ILivingBeing
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Camera and movement
	UPROPERTY(EditAnywhere)
	class UCameraComponent* PlayerCamera;
	UPROPERTY(EditAnywhere)
	class USpringArmComponent* CameraArm;

	// Movement
	void MoveForward(float Value);
	void MoveRight(float Value);

	// Stamina (Stamina variable has already been declared in LivingBeing.h)
	FTimerHandle StaminaDelayHandle;
	StatStatus StaminaStatus;
	void RegenerateStamina();
	void DrainStamina();
	void ToggleSprinting();
	bool bIsSprinting;
	bool bCanSprint;






	// Behavior
	virtual void Die() override;

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
