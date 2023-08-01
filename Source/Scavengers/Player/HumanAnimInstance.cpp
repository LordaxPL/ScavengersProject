// Fill out your copyright notice in the Description page of Project Settings.


#include "HumanAnimInstance.h"
#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UHumanAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (Player == nullptr)
	{
		Player = Cast<APlayerCharacter>(TryGetPawnOwner());
		bIsGoingBackward = false;
	}
	
}

void UHumanAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (Player != nullptr)
	{
		FVector Velocity = Player->GetVelocity();
		Velocity.Z = 0.0f;
		Speed = Velocity.Size();

		bIsInAir = Player->GetCharacterMovement()->IsFalling();

		Direction = UAnimInstance::CalculateDirection(Velocity, Player->GetActorRotation());
		
		bShouldTurnLeft = Player->bShouldTurnLeft;
		bShouldTurnRight = Player->bShouldTurnRight;

		if (Direction > 160.0f || Direction < -160.0f)
		{
			bIsGoingBackward = true;
		}
		else
		{
			bIsGoingBackward = false;
		}
	}

}