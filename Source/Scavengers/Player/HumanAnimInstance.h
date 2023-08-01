// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "HumanAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SCAVENGERS_API UHumanAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class APlayerCharacter* Player;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsInAir;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Direction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bShouldTurnLeft;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bShouldTurnRight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsGoingBackward;

	
};
