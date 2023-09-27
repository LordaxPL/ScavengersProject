// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "UIHandler.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Scavengers/Environment/Pickable.h"
#include "Scavengers/Environment/Interactable.h"
#include "Scavengers/Player/Inventory.h"
#include "Scavengers/Environment/Openable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Scavengers/Environment/MeleeWeapon.h"

#define Print(String) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, String);

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Camera
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(RootComponent);
	ZoomProgress = 0.0f;

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
	bFreezed = false;

	// Stamina and sprinting
	StaminaStatus = Stable;
	bIsSprinting = false;
	bCanSprint = true;
	StaminaRegenerationStrength = 0.125f;

	// Crouching
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchingSpeed;

	// UI
	UIHandler = CreateDefaultSubobject<UUIHandler>(TEXT("UIHandler"));

	// Inventory
	Inventory = CreateDefaultSubobject<UInventory>(TEXT("Inventory"));

	// Parkour
	bIsClimbing = false;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
	static ConstructorHelpers::FObjectFinder<UAnimMontage> ParkourMontageObject(TEXT("AnimMontage'/Game/Player/Animation/AM_ParkourMontage.AM_ParkourMontage'"));
	if (ParkourMontageObject.Succeeded())
	{
		ParkourMontage = ParkourMontageObject.Object;
	}

	// Interaction
	static ConstructorHelpers::FObjectFinder<UAnimMontage> InteractionMontageObject(TEXT("AnimMontage'/Game/Player/Animation/AM_Collect.AM_Collect'"));
	if (InteractionMontageObject.Succeeded())
	{
		InteractionMontage = InteractionMontageObject.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> LockpickingMontageObject(TEXT("AnimMontage'/Game/Player/Animation/AM_Lockpicking.AM_Lockpicking'"));
	if (LockpickingMontageObject.Succeeded())
	{
		LockpickingMontage = LockpickingMontageObject.Object;
	}

	InteractionProgress = 0.0f;
	InteractablesDetectionRadius = 250.0f;
	InteractablesDetector = CreateDefaultSubobject<USphereComponent>(TEXT("InteractablesDetector"));
	InteractablesDetector->SetupAttachment(RootComponent);
	InteractablesDetector->SetSphereRadius(InteractablesDetectionRadius);
	InteractablesDetector->SetHiddenInGame(false);
	bCanInteract = true;
	bAttemptedLockpicking = false;

}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetMesh()->GetAnimInstance()->OnMontageEnded.AddDynamic(this, &APlayerCharacter::OnMontageEnded);
	GetMesh()->GetAnimInstance()->OnMontageBlendingOut.AddDynamic(this, &APlayerCharacter::OnMontageBlendingOut);
	GetMesh()->GetAnimInstance()->OnMontageStarted.AddDynamic(this, &APlayerCharacter::OnMontageStarted);
	InteractablesDetector->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::DetectInteractable);
	InteractablesDetector->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::ForgetInteractable);
	PlayerController = Cast<APlayerController>(GetController());
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Tick has been turned off in the constructor
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

	PlayerInputComponent->BindAction(FName("Interact"), IE_Pressed, this, &APlayerCharacter::Interact);
	PlayerInputComponent->BindAction(FName("Interact"), IE_Released, this, &APlayerCharacter::StopInteracting);
	PlayerInputComponent->BindAction(FName("Inventory"), IE_Pressed, this, &APlayerCharacter::ToggleInventory);
	PlayerInputComponent->BindAction(FName("SwitchWeaponUp"), IE_Pressed, this, &APlayerCharacter::SwitchWeaponUp);
	PlayerInputComponent->BindAction(FName("SwitchWeaponDown"), IE_Pressed, this, &APlayerCharacter::SwitchWeaponDown);


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

	if (Value < -0.05f)
	{
		bShouldTurnLeft = true;
		bShouldTurnRight = false;
	}
	else if (Value > 0.05f)
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
	if (bFreezed)
	{
		bIsSprinting = false;
		InitStaminaRegeneration();
		return;
	}
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
	if (bCanSprint)
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
}

void APlayerCharacter::ToggleCrouching()
{
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
				Stamina -= 10.0f;
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
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_GameTraceChannel1, CollisionParams);
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

	GetWorld()->LineTraceSingleByChannel(HitResult, HeightCheckStart, HeightCheckEnd, ECollisionChannel::ECC_GameTraceChannel1, CollisionParams);

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
		GetWorld()->SweepSingleByChannel(HitResult, SpaceCheckStart, SpaceCheckEnd, FQuat::Identity, ECollisionChannel::ECC_GameTraceChannel1, CapsuleShape, CollisionParams);
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
		GetWorld()->SweepSingleByChannel(HitResult, ThickCheckStart, ThickCheckEnd, FQuat::Identity, ECollisionChannel::ECC_GameTraceChannel1, BoxShape, CollisionParams);
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
	else if (Montage == InteractionMontage)
	{
		bCanInteract = false;
	}
}

void APlayerCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == InteractionMontage)
	{
		bCanInteract = true;
	}
}

void APlayerCharacter::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
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

void APlayerCharacter::DetectInteractable(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	/*
	FString Text = FString::Printf(TEXT("Detected: %s"), *OtherActor->GetName());
	Print(Text);
	*/

	if (Cast<IInteractable>(OtherActor))
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, OtherActor->GetName());
		Interactables.Add(OtherActor);
	}
}

void APlayerCharacter::ForgetInteractable(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	/*
	FString Text = FString::Printf(TEXT("Lost: %s"), *OtherActor->GetName());
	Print(Text);
	*/

	if (Cast<IInteractable>(OtherActor))
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, OtherActor->GetName());
		Interactables.Remove(OtherActor);
	}
}

void APlayerCharacter::Interact()
{
	if (bCanInteract)
	{
		PlayAnimMontage(InteractionMontage);

		FVector CameraLocation = PlayerCamera->GetComponentLocation();

		float CurrentDot = 0.0f;

		// How far the player can be to open a door [cm]
		float MaxDistanceToOpen = 120.0f;

		//Determines how far away from the object the player can point to detect it
		//BestDot = 1.0f means the player has to point exactly at the location of the object
		float BestDot = 0.995f;

		AActor* LookAtActor = nullptr;

		// Detecting by forward vector
		FHitResult ForwardHitResult;
		FCollisionQueryParams ColParams;
		ColParams.AddIgnoredActor(this);
		FVector EndVec = GetActorLocation() + GetActorForwardVector() * MaxDistanceToOpen;
		DrawDebugLine(GetWorld(), GetActorLocation(), EndVec, FColor::Purple, true);

		if (GetWorld()->LineTraceSingleByChannel(ForwardHitResult, GetActorLocation(), EndVec, ECollisionChannel::ECC_Visibility, ColParams))
		{
			LookAtActor = ForwardHitResult.GetActor();
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, ForwardHitResult.Actor->GetName());
		}


		// Detecting by Dot Product
		for (AActor* Actor : Interactables)
		{
			FVector LookAtVec = UKismetMathLibrary::FindLookAtRotation(CameraLocation, Actor->GetActorLocation()).Vector();
			FString Name = Actor->GetName();

			CurrentDot = FVector::DotProduct(GetControlRotation().Vector(), LookAtVec);
			if (CurrentDot > BestDot)
			{
				BestDot = CurrentDot;
				LookAtActor = Actor;
			}
		}

		// Detecting by LineTrace
		if (LookAtActor == nullptr)
		{
			FHitResult InteractionHitResult;
			FVector End = CameraLocation;
			End += GetControlRotation().Vector() * InteractablesDetectionRadius * 2;
			DrawDebugBox(GetWorld(), End, FVector(5.0f), FColor::Red, true);
			if (GetWorld()->LineTraceSingleByChannel(InteractionHitResult, CameraLocation, End, ECollisionChannel::ECC_Visibility))
			{
				for (AActor* Actor : Interactables)
				{
					if (InteractionHitResult.Actor == Actor)
					{
						LookAtActor = Cast<AActor>(InteractionHitResult.Actor);
					}
				}
			}
		}

		if (LookAtActor != nullptr)
		{
			FString Name = LookAtActor->GetName();

			if (AMeleeWeapon* Melee = Cast<AMeleeWeapon>(LookAtActor))
			{
				uint8 SlotID = Inventory->AddWeapon(Melee->ItemID);
				if (SlotID == 4)
				{
					// Weapon was added to the inventory
				}
				else
				{
					// Weapon was added to a slot
					UIHandler->SetWeaponSlotImage(SlotID, Melee->ItemID);
					if (EquippedWeapon == nullptr)
					{
						Melee->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), FName("RightHandSocket"));
						EquippedWeapon = Melee;
					}
					else
					{
						Melee->Destroy();
					}
				}
				Melee->Interact();
			}
			else if (APickable* Pickable = Cast<APickable>(LookAtActor))
			{
				UIHandler->ShowNotification(Name);
				Pickable->Interact();
				PlayAnimMontage(InteractionMontage);
				Inventory->AddItem(Pickable->ItemID);
				if (Inventory->IsVisible())
				{
					Inventory->RefreshInventoryWidget();
				}

				if (Inventory->IsOverencumbered())
				{
					GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, "PLAYER IS OVERENCUMBERED!");
				}
			}
			else if (AOpenable* Openable = Cast<AOpenable>(LookAtActor))
			{
				// check if the player has a key

				float Distance = FVector::Distance(GetActorLocation(), Openable->GetActorLocation());

				if (FVector::Distance(GetActorLocation(), LookAtActor->GetActorLocation()) <= MaxDistanceToOpen)
				{
					if (Openable->IsLocked())
					{
						CurrentOpenable = Openable;

						// If the progress bar is not being lerped already, 
						// check if after 0.125 second the F key is still being hold, if so, start lerping the progress bar
						if (!GetWorldTimerManager().IsTimerActive(ProgressLerpHandle))
						{
							InteractionProgress = 0.0f;
							GetWorldTimerManager().SetTimer(ProgressLerpHandle, this, &APlayerCharacter::InteractionHold, 0.125f, false);
						}
					}
					else
					{
						PlayAnimMontage(InteractionMontage);
						CurrentOpenable = nullptr;
						Openable->Interact();
					}

				}
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, "Looking at nothing");
		}
	}
	else
	{
		Print("CANT INTERACT");
	}
}

void APlayerCharacter::StopInteracting()
{
	if (CurrentOpenable != nullptr)
	{
		if (bAttemptedLockpicking)
		{
			UIHandler->ToggleProgressBar(false);
			GetMesh()->GetAnimInstance()->Montage_Stop(0.5f, LockpickingMontage);
			if (GetWorldTimerManager().IsTimerActive(ProgressLerpHandle))
			{
				GetWorldTimerManager().ClearTimer(ProgressLerpHandle);
				InteractionProgress = 0.0f;
			}
			FocusCamera(false);
			bAttemptedLockpicking = false;
			TogglePlayerFreeze(false);
		}
		else
		{
			// ADD A SOUND HERE WHICH INDICATES THE DOOR IS LOCKED
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, "THE DOOR IS LOCKED - ADD FUNCTIONALITY HERE:");
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, __FUNCTION__);
		}
		CurrentOpenable = nullptr;
	}
}

void APlayerCharacter::SetInputMode(bool bIsUI)
{

	// Shows mouse cursor and places it in the middle of the screen
	if (PlayerController)
	{
		if (bIsUI)
		{
			FVector2D MiddlePoint = GetScreenMiddlePoint();
			PlayerController->SetMouseLocation(MiddlePoint.X, MiddlePoint.Y);
			PlayerController->SetInputMode(FInputModeGameAndUI());
			PlayerController->bShowMouseCursor = true;
		}
		else
		{
			PlayerController->SetInputMode(FInputModeGameOnly());
			PlayerController->bShowMouseCursor = false;
		}
	}
}

void APlayerCharacter::ToggleInventory()
{
	if (Inventory != nullptr)
	{
		SetInputMode(Inventory->ToggleInventory());
	}
}

FVector2D APlayerCharacter::GetScreenMiddlePoint() const
{
	if (PlayerController)
	{
		FViewport* Viewport = PlayerController->GetLocalPlayer()->ViewportClient->Viewport;
		if (Viewport)
		{
			FVector2D ViewportSize = Viewport->GetSizeXY();
			ViewportSize *= 0.5f;
			return ViewportSize;

		}
		return FVector2D::ZeroVector;
	}
	else
	{
		return FVector2D::ZeroVector;
	}
}

void APlayerCharacter::LerpProgress()
{
	InteractionProgress += 0.01f;
	UIHandler->SetProgressBar(InteractionProgress);
	if (InteractionProgress >= 1.0f)
	{
		GetWorldTimerManager().ClearTimer(ProgressLerpHandle);
		UIHandler->SwitchInteractionImage(2);
		InteractionProgress = 0.0f;
		GetWorldTimerManager().SetTimer(ProgressLerpHandle, this, &APlayerCharacter::EndInteractionSequence, 0.625f, false);
	}
}

void APlayerCharacter::EndInteractionSequence()
{
	//UIHandler->ToggleProgressBar(false);
	//GetMesh()->GetAnimInstance()->Montage_Stop(0.5f, LockpickingMontage);
	if (CurrentOpenable != nullptr)
	{
		CurrentOpenable->Interact();
	}
	StopInteracting();
}

void APlayerCharacter::InteractionHold()
{
	if (PlayerController->IsInputKeyDown(FName("F")))
	{
		bAttemptedLockpicking = true;
		PlayAnimMontage(LockpickingMontage);
		UIHandler->ToggleProgressBar(true);
		GetWorldTimerManager().SetTimer(ProgressLerpHandle, this, &APlayerCharacter::LerpProgress, 0.05f, true);
		FocusCamera(true);
		TogglePlayerFreeze(true);
	}
	else
	{
		PlayAnimMontage(InteractionMontage, 1.5f);
	}
}

void APlayerCharacter::FocusCamera(bool bZoomIn)
{
	if (GetWorldTimerManager().IsTimerActive(CameraLerpHandle))
	{
		GetWorldTimerManager().ClearTimer(CameraLerpHandle);
	}

	if (bZoomIn)
	{
		GetWorldTimerManager().SetTimer(CameraLerpHandle, this, &APlayerCharacter::LerpCameraZoomIn, 0.03125f, true);
	}
	else
	{
		GetWorldTimerManager().SetTimer(CameraLerpHandle, this, &APlayerCharacter::LerpCameraZoomOut, 0.03125f, true);
	}
}

void APlayerCharacter::LerpCameraZoomIn()
{
	ZoomProgress += 0.0625f;
	CameraArm->TargetArmLength = FMath::Lerp(250.0f, 125.0f, ZoomProgress);
	if (ZoomProgress >= 1.0f)
	{
		GetWorldTimerManager().ClearTimer(CameraLerpHandle);
	}
}

void APlayerCharacter::LerpCameraZoomOut()
{
	ZoomProgress -= 0.0625f;
	CameraArm->TargetArmLength = FMath::Lerp(250.0f, 125.0f, ZoomProgress);
	if (ZoomProgress <= 0.0f)
	{
		GetWorldTimerManager().ClearTimer(CameraLerpHandle);
	}

}

void APlayerCharacter::TogglePlayerFreeze(bool bFreeze)
{
	if (bFreezed == bFreeze)
	{
		return;
	}

	if (bFreeze)
	{
		GetCharacterMovement()->MaxWalkSpeed = 0.0f;
		GetCharacterMovement()->MaxWalkSpeedCrouched = 0.0f;
		GetCharacterMovement()->RotationRate = FRotator(0.0f);
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed;
		GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchingSpeed;
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
	}
	bFreezed = bFreeze;
}

void APlayerCharacter::SwitchWeaponUp()
{
	EquipWeapon(Inventory->SwitchWeapon(true));
	UIHandler->SwitchWeapon(true);
}

void APlayerCharacter::SwitchWeaponDown()
{
	EquipWeapon(Inventory->SwitchWeapon(false));
	UIHandler->SwitchWeapon(false);
}

void APlayerCharacter::EquipWeapon(uint8 WeaponID)
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
	if (WeaponID != 0)
	{
		EquippedWeapon = Inventory->SpawnWeapon(WeaponID);
		EquippedWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), FName("RightHandSocket"));
		EquippedWeapon->Interact();
	}
	else
	{
		EquippedWeapon = nullptr;
	}
	

}