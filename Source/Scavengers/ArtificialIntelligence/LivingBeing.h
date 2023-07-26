// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LivingBeing.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULivingBeing : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SCAVENGERS_API ILivingBeing
{
	GENERATED_BODY()

		// Add interface functions to this class. This is the class that will be inherited to implement this interface.
protected:
		virtual void Die();
		float Health = 100;
		float MaxHealth = Health;
		float Stamina = 100;
		float MaxStamina = Stamina;

		float WalkingSpeed = 150.0f;
		float SprintingSpeed = 300.0f;
		float CrouchingSpeed = 75.0f;
public:
	inline float GetHealth() const { return Health; }
	inline float GetMaxHealth() const { return MaxHealth; }
	inline float GetStamina() const { return Stamina; }
	inline float GetMaxStamina() const { return MaxStamina; }
};
