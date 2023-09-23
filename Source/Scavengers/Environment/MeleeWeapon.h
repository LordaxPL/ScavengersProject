// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Scavengers/Environment/Pickable.h"
#include "MeleeWeapon.generated.h"

/**
 * 
 */
UCLASS()
class SCAVENGERS_API AMeleeWeapon : public APickable
{
	GENERATED_BODY()
public:
		AMeleeWeapon();
		bool Hit();
		bool HitHard();
		virtual void Interact() override;
};
