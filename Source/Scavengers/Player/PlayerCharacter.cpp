// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "UIHandler.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#define Print(String) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, String);

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

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

	CapsuleComp = Cast<UCapsuleComponent>(GetRootComponent());

	// movement
	bShouldTurnLeft = false;
	bShouldTurnRight = false;

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

	// Parkour
	bIsClimbing = false;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
	static ConstructorHelpers::FObjectFinder<UAnimMontage> ParkourMontageObject(TEXT("AnimMontage'/Game/Player/Animation/AM_ParkourMontage.AM_ParkourMontage'"));
	if (ParkourMontageObject.Succeeded())
	{
		ParkourMontage = ParkourMontageObject.Object;
	}

	// Interaction
	SearchRadius = 150.0f;
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Interaction Sphere"));
	InteractionSphere->SetSphereRadius(SearchRadius);
	InteractionSphere->SetHiddenInGame(false);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetMesh()->GetAnimInstance()->OnMontageEnded.AddDynamic(this, &APlayerCharacter::OnMontageEnded);
	GetMesh()->GetAnimInstance()->OnMontageStarted.AddDynamic(this, &APlayerCharacter::OnMontageStarted);

	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::DetectInteractable);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::ForgetInteractable);

}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Ticking has been disabled in the constructor
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &APlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis(FName("LookRight"), this, &APlayerCharacter::TurnLeftRight);
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

void APlayerCharacter::TurnLeftRight(float Value)
{
	APawn::AddControllerYawInput(Value);

	if (bIsCrouched || GetVelocity().Size() != 0.0f)
	{
		return;
	}

	if (Value < -0.3f)
	{
		bShouldTurnLeft = true;
		bShouldTurnRight = false;
	}
	else if (Value > 0.3f)
	{
		bShouldTurnLeft = false;
		bShouldTurnRight = true;
	}
	else
	{
		bShouldTurnLeft = false;
		bShouldTurnRight = false;
	}
}

void APlayerCharacter::ToggleSprinting()
{
	Stamina < 25.0f ? bCanSprint = false : bCanSprint = true;

	if (bCanSprint && !bIsSprinting)
	{
		bIsSprinting = true;

		// start sprinting
		if (bIsCrouched)
		{
			ToggleCrouching();
		}
		else
		{
			GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
			InterruptStaminaRegeneration();
			InitStaminaDraining();
		}
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

void APlayerCharacter::InitStaminaDraining()
{
	if (StaminaStatus != Draining)
	{
		if (GetWorldTimerManager().IsTimerActive(StaminaDelayHandle))
		{
			GetWorldTimerManager().ClearTimer(StaminaDelayHandle);
		}
		StaminaStatus = Draining;
		GetWorldTimerManager().SetTimer(StaminaDelayHandle, this, &APlayerCharacter::DrainStamina, 0.1f, true, 0.0f);
	}
}

void APlayerCharacter::InterruptStaminaRegeneration()
{
	if (GetWorldTimerManager().IsTimerActive(StaminaDelayHandle) && StaminaStatus == Regenerating)
	{
		GetWorldTimerManager().ClearTimer(StaminaDelayHandle);
		StaminaStatus = Stable;
	}
}

void APlayerCharacter::InterruptStaminaDraining()
{
	if (GetWorldTimerManager().IsTimerActive(StaminaDelayHandle) && StaminaStatus == Draining)
	{
		GetWorldTimerManager().ClearTimer(StaminaDelayHandle);
		StaminaStatus = Stable;
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
		if (bCanSprint)
		{
			Stamina -= 0.25f;
			UIHandler->AdjustStaminaBar(Stamina);
		}
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
	// TEMPORARY!!!
	ScanForInteractables();

	if (bIsClimbing)
	{
		return;
	}

	if (bIsCrouched)
	{
		if (CanUnCrouch())
		{
			UnCrouch();
			StaminaRegenerationStrength *= 0.5f;

			// If the sprinting button wasn't released during the crouch
			if (bIsSprinting)
			{
				GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
				InterruptStaminaRegeneration();
				InitStaminaDraining();
			}
		}
	}
	else
	{
		Crouch();
		StaminaRegenerationStrength *= 2.0f;
		InterruptStaminaDraining();
		InitStaminaRegeneration();
	}
}

bool APlayerCharacter::CanUnCrouch()
{
	FVector Start = GetActorLocation();
	Start.Z += 28.0f;
	FHitResult SweepResult;
	FCollisionQueryParams ColParams;
	ColParams.AddIgnoredActor(this);
	FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(FVector(34.0f, 34.0f, 88.0f));
	return !GetWorld()->SweepSingleByChannel(SweepResult, Start, Start + FVector(0.1f), FQuat::Identity, ECollisionChannel::ECC_WorldStatic, CapsuleShape, ColParams);
	
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

	if (!bIsClimbing)
	{
		if (Stamina > 20.0f && !GetCharacterMovement()->IsFalling() && !bIsClimbing)
		{
			// if climb was successful
			if (AttemptClimb())
			{
				InterruptStaminaRegeneration();
				StaminaStatus = Stable;
				InitStaminaRegeneration();
				Stamina -= 20.0f;
				UIHandler->AdjustStaminaBar(Stamina);
			}
			else if (!bIsClimbing, !GetCharacterMovement()->IsCrouching())
			{
				Super::Jump();
				InterruptStaminaRegeneration();
				StaminaStatus = Stable;
				InitStaminaRegeneration();
				Stamina -= 20.0f;
				UIHandler->AdjustStaminaBar(Stamina);
			}
		}

		if (bIsSprinting)
		{
			// If the player is still able to sprint after jumping or climbing, drain stamina
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

}

bool APlayerCharacter::AttemptClimb()
{
	if (bIsClimbing)
	{
		return false;
	}

	bool bThick = false;
	bool bIsPlayerCrouching = GetCharacterMovement()->IsCrouching();

	// Z when standing = 90.0f
	// Z when crouching = 62.0f

	// Decision on what animation to play
	uint8 Decision = 0;
	// 1 - climb
	// 2 - get up a ledge
	// 3 - climb up and jump down
	// 4 - vault

	// Finding a climbable object
	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	FVector Start = GetActorLocation();
	bIsPlayerCrouching ? Start.Z -= 18.0f : Start.Z -= 46.0f;

	FVector End = Start + GetActorForwardVector() * 100.0f;
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_WorldStatic, CollisionParams);
	FVector ObjectNormal = HitResult.ImpactNormal;
	FVector ObjectStart = HitResult.ImpactPoint;
	if (HitResult.bBlockingHit)
	{
		// Uncomment to draw parkour detection debug line
		//DrawDebugLine(GetWorld(), Start, HitResult.ImpactPoint, FColor::Yellow, true, -1.0f, 0U, 1.0f);
	}
	else
	{
		return false;
	}

	// Finding height of the object
	FVector HeightCheckStart = HitResult.Location;
	HeightCheckStart += HitResult.ImpactNormal * -10.0f;
	FVector HeightCheckEnd = HeightCheckStart;
	HeightCheckStart.Z += 200.0f;

	GetWorld()->LineTraceSingleByChannel(HitResult, HeightCheckStart, HeightCheckEnd, ECollisionChannel::ECC_WorldStatic, CollisionParams);

	if (HitResult.ImpactPoint == FVector::ZeroVector)
	{
		return false;
	}

	// Checking if the player can climb the object (if the object is not too tall)
	float HeightToClimb = HitResult.Location.Z - GetActorLocation().Z;

	// Correcting the value depending on whether the player is crouching or not
	bIsPlayerCrouching ? HeightToClimb += 60.0f : HeightToClimb += 88.0f;
	
	float ObjectHeight = HitResult.Location.Z;

	if (HeightToClimb < 200.0f)
	{

		// Checking if there's enough space on top of the object
		FVector SpaceCheckStart = HitResult.Location;
		SpaceCheckStart.Z += 95.0f;
		FVector SpaceCheckEnd = SpaceCheckStart + FVector(0.1f);
		FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(FVector(34.0f, 34.0f, 88.0f));
		GetWorld()->SweepSingleByChannel(HitResult, SpaceCheckStart, SpaceCheckEnd, FQuat::Identity, ECollisionChannel::ECC_WorldStatic, CapsuleShape, CollisionParams);
		if (HitResult.bBlockingHit)
		{
			return false;
		}

		// Checking if the object is thick
		FVector ThickCheckEnd = ObjectStart + ObjectNormal * -20.0f;
		ThickCheckEnd.Z = ObjectHeight;
		FVector ThickCheckStart = ThickCheckEnd;
		ThickCheckStart.Z += 20.0f;
		ThickCheckEnd.Z -= 20.0f;
		FCollisionShape BoxShape = FCollisionShape::MakeBox(FVector(5.0f));
		GetWorld()->SweepSingleByChannel(HitResult, ThickCheckStart, ThickCheckEnd, FQuat::Identity, ECollisionChannel::ECC_WorldStatic, BoxShape, CollisionParams);
		if (HitResult.bBlockingHit)
		{
			bThick = true;
		}

		

	}
	else
	{
		return false;
	}

	Print(FString::SanitizeFloat(HeightToClimb))

	if (HeightToClimb < 105.0f)
	{
		bThick ? Decision = 2 : Decision = 4;
	}
	else
	{
		bThick ? Decision = 1 : Decision = 3;
	}

	bIsClimbing = true;
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Motion wrapping
	FVector AdjustedPlayerLocation = ObjectStart + ObjectNormal * 50.0f;
	if (Decision == 1 || Decision == 3)
	{
		AdjustedPlayerLocation.Z = ObjectHeight - 70.0f;
	}
	else
	{
		AdjustedPlayerLocation.Z = ObjectHeight - 20.0f;
	}
	SetActorLocation(AdjustedPlayerLocation);


	switch (Decision)
	{

		// 1 - climb
		// 2 - get up a ledge
		// 3 - climb up and jump down
		// 4 - vault

	case 1:
		GetMesh()->GetAnimInstance()->Montage_Play(ParkourMontage);
		break;
	case 2:
		GetMesh()->GetAnimInstance()->Montage_Play(ParkourMontage);
		GetMesh()->GetAnimInstance()->Montage_JumpToSection(FName("GettingUp"), ParkourMontage);
		break;
	case 3:
		GetMesh()->GetAnimInstance()->Montage_Play(ParkourMontage);
		GetMesh()->GetAnimInstance()->Montage_JumpToSection(FName("JumpingDown"), ParkourMontage);
		break;
	case 4:
		GetMesh()->GetAnimInstance()->Montage_Play(ParkourMontage);
		GetMesh()->GetAnimInstance()->Montage_JumpToSection(FName("Vaulting"), ParkourMontage);
		break;
	default:
		break;
	}

	return true;

}

void APlayerCharacter::OnMontageStarted(UAnimMontage* Montage)
{
	if (Montage == ParkourMontage)
	{
		if (CapsuleComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
		{
			CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		FString Name = Montage->GetName();
		Print(FString::Printf(TEXT("Started montage: %s"), *Name));
		bIsClimbing = true;

		bCanSprint = false;
	}
}

void APlayerCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == ParkourMontage)
	{
		if (CapsuleComp->GetCollisionEnabled() != ECollisionEnabled::QueryAndPhysics)
		{
			CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}

		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		FString Name = Montage->GetName();
		Print(FString::Printf(TEXT("Ended montage: %s"), *Name));
		bIsClimbing = false;

		bCanSprint = true;
	}
}

void APlayerCharacter::ScanForInteractables()
{
	FCollisionQueryParams ColParams;
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0.1f);

	ColParams.AddIgnoredActor(this);
	GetWorld()->SweepMultiByChannel(
		InteractablesNearby,
		Start,
		End,
		FQuat::Identity,
		ECollisionChannel::ECC_WorldStatic,
		FCollisionShape::MakeBox(FVector(SearchRadius)),
		ColParams
	);

	TArray<AActor*> DetectedInteractables;

	DrawDebugBox(GetWorld(), Start, FVector(SearchRadius), FColor::Blue, true);
	DrawDebugBox(GetWorld(), End, FVector(SearchRadius), FColor::Red, true);
	for (FHitResult HitResult : InteractablesNearby)
	{
		DrawDebugBox(GetWorld(), HitResult.ImpactPoint, FVector(10.0f), FColor::Purple, true, -1.0f, 0U, 5.0f);

		FString Name = HitResult.Actor->GetName();
		UE_LOG(LogTemp, Warning, TEXT("Name of the actor: %s"), *Name);
	}
}

void APlayerCharacter::DetectInteractable(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	Print("+++");
}

void APlayerCharacter::ForgetInteractable(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	Print("---");
}
