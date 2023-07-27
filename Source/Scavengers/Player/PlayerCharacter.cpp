// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "UIHandler.h"
#include "GameFramework/CharacterMovementComponent.h"

#define Print(String) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, String);

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(RootComponent);

	PlayerCamera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);

	CameraArm->bUsePawnControlRotation = true;
	PlayerCamera->bUsePawnControlRotation = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->MaxWalkSpeed = 150.0f;

	// Stamina and sprinting
	StaminaStatus = Stable;
	bIsSprinting = false;
	bCanSprint = true;
	StaminaRegenerationStrength = 0.125f;

	// Crouching
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 100.0f;

	// UI
	UIHandler = CreateDefaultSubobject<UUIHandler>(TEXT("UIHandler"));



}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &APlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis(FName("LookRight"), this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis(FName("LookUp"), this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction(FName("Jump"), IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(FName("Sprint"), IE_Pressed, this, &APlayerCharacter::ToggleSprinting);
	PlayerInputComponent->BindAction(FName("Sprint"), IE_Released, this, &APlayerCharacter::ToggleSprinting);
	PlayerInputComponent->BindAction(FName("Crouch"), IE_Pressed, this, &APlayerCharacter::ToggleCrouching);

}

void APlayerCharacter::Die()
{
	Print("Implement dying");
}


void APlayerCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		FRotator Rotation = Controller->GetControlRotation();
		Rotation.Pitch = 0.0f;
		Rotation.Roll = 0.0f;
		FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void APlayerCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		FRotator Rotation = Controller->GetControlRotation();
		Rotation.Pitch = 0.0f;
		Rotation.Roll = 0.0f;
		FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void APlayerCharacter::ToggleSprinting()
{
	Stamina < 25.0f ? bCanSprint = false : bCanSprint = true;

	if (bCanSprint && !bIsSprinting)
	{
		bIsSprinting = true;
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;

		// start sprinting
		InterruptStaminaRegeneration();
		StaminaStatus = Draining;
		GetWorldTimerManager().SetTimer(StaminaDelayHandle, this, &APlayerCharacter::DrainStamina, 0.1f, true, 0.0f);
	}
	else
	{
		if (bIsSprinting)
		{
			bIsSprinting = false;
		}
		GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed;

		InitStaminaRegeneration();
	}
}

void APlayerCharacter::InitStaminaRegeneration()
{
	if (StaminaStatus != Regenerating)
	{
		if (GetWorldTimerManager().IsTimerActive(StaminaDelayHandle))
		{
			GetWorldTimerManager().ClearTimer(StaminaDelayHandle);
		}
		StaminaStatus = Regenerating;
		GetWorldTimerManager().SetTimer(StaminaDelayHandle, this, &APlayerCharacter::RegenerateStamina, 0.1f, true, 3.0f);
	}
}

void APlayerCharacter::InterruptStaminaRegeneration()
{
	if (GetWorldTimerManager().IsTimerActive(StaminaDelayHandle))
	{
		GetWorldTimerManager().ClearTimer(StaminaDelayHandle);
	}
}

void APlayerCharacter::RegenerateStamina()
{
	Print(FString::SanitizeFloat(Stamina));
	if (Stamina < MaxStamina)
	{
		Stamina += StaminaRegenerationStrength;
		UIHandler->AdjustStaminaBar(Stamina);
	}
	else
	{
		StaminaStatus = Stable;
		Stamina = MaxStamina;
		bCanSprint = true;
		UIHandler->AdjustStaminaBar(Stamina);
		GetWorldTimerManager().ClearTimer(StaminaDelayHandle);
	}
}

void APlayerCharacter::DrainStamina()
{

	if (Stamina > 0)
	{
		Stamina -= 0.25f;
		UIHandler->AdjustStaminaBar(Stamina);
	}
	else
	{
		Stamina = 0.0f;
		UIHandler->AdjustStaminaBar(Stamina);
		GetWorldTimerManager().ClearTimer(StaminaDelayHandle);
		GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed;
		StaminaStatus = Stable;
	}
}

void APlayerCharacter::ToggleCrouching()
{
		if (bIsCrouched)
		{
			UnCrouch();
			StaminaRegenerationStrength *= 0.5f;

		}
		else
		{
			Crouch();
			StaminaRegenerationStrength *= 2.0f;
		}
}

float APlayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);
	if (Health == 0)
	{
		Die();
	}
	Print(FString::SanitizeFloat(Health));
	UIHandler->AdjustHealthBar(Health);
	return DamageAmount;

}

void APlayerCharacter::Jump()
{

	if (Stamina > 20.0f && !GetCharacterMovement()->IsFalling() && !GetCharacterMovement()->IsCrouching())
	{
		InterruptStaminaRegeneration();
		StaminaStatus = Stable;
		InitStaminaRegeneration();
		Super::Jump();
		Stamina -= 20.0f;
		UIHandler->AdjustStaminaBar(Stamina);
	}

	if (bIsSprinting)
	{
		// If the player is still able to sprint after jumping, drain stamina
		StaminaStatus = Draining;
		if (GetWorldTimerManager().IsTimerActive(StaminaDelayHandle))
		{
			GetWorldTimerManager().ClearTimer(StaminaDelayHandle);
		}
		GetWorldTimerManager().SetTimer(StaminaDelayHandle, this, &APlayerCharacter::DrainStamina, 0.1f, true, 0.0f);

		if (Stamina < 25.0f)
		{
			// If player was sprinting and got tired by jumping, turn off sprinting
			ToggleSprinting();
		}
	}

}