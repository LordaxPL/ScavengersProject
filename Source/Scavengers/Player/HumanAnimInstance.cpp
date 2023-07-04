// Fill out your copyright notice in the Description page of Project Settings.


#include "HumanAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UHumanAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (Player == nullptr)
	{
		Player = Cast<ACharacter>(TryGetPawnOwner());
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


	}

}